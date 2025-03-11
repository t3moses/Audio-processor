
#include "mf_types.h"
#include "mf_audio_sampler.h"
#include "mf_dft.h"
#include "mf_dewindow.h"
#include "mf_quorum.h"
#include "mf_sieve.h"
#include "mf_note.h"
#include "mf_dynamic.h"
#include "mf_channel.h"
#include "mf_midi.h"

#include <mf_trigger.h>
#define SCOPE_TRIGGER         15   // Connect an oscilloscope trigger to this GPIO.
#include <mf_dac.h>
#define SCOPE_CH_1            17   // Connect an oscilloscope analog channel to this GPIO.

#define DFT_BOUND             36   // Three octaves, inclusive.
#define DEWINDOW_BOUND         2   // DEWINDOW_BOUND + 1 is the order of the dewindowing matrix.
#define A2                   110.0 // Frequency in Hz of note A2.
#define REF_CYCLES            10   // The sampling window contains ( REF_CYCLES + 1 ) x ( REF_DATAPOINTS + 1 ) reference datapoints.
#define REF_DATAPOINTS        15
#define DFT_ATTEN            150   // Division converts audio to DFT value.
#define DFT_ROOT_NOTE         -5   // Degree 0 relative to A2 in semitones.  -5 is equivalent to E2.

#define BEND_MESSAGE_BITS    ( 1 << 14 ) // The MIDI pitch bend message allocates 14 bits for the bend value.
#define BEND_RANGE           400         // Total range of pitch bend in cent.  400 is +/- 2 semitones.
#define POLE                   0.4       // Phase-locked loop pole x coordinate.  0.0 < POLE < 1.0.  Lower values result in faster response.

#define ADC_CS                 0 // GPIO number = Saola 1 board J1-2.  AD7042-4
#define ADC_MISO               1 // GPIO number = Saola 1 board J1-3.  AD7042-3
#define ADC_CLK                2 // GPIO number = Saola 1 board J1-4.  AD7042-2
#define ADC_BITS              12 // ADC resolution in bits.

#define QUORUM_LIMIT           0.8 // Cumulative proportion of total energy to etablish quorum.
#define QUORUM_THRESHOLD      20   // ai_quorum is empty unless i_dft_peak exceeds this value.

#define MIDI_OUT                4 // Transmit MIDI on this GPIO pin.
#define MIDI_IN                 5 // Receive MIDI on this GPIO pin.
#define CHANNEL_BASE           00 // Set the KORG N5EX or Roland SPD20 channel to MIDI_CHANNEL + 1
#define CHANNEL_BOUND           5 // The KORG N5EX or Roland SPD20 uses channels CHANNEL_BASE + 0 to CHANNEL_BASE + CHANNEL_BOUND
#define ROOT_KEY               45 // MIDI key equivalent to A2.
#define KEY_FROM_DEGREE        ( ROOT_KEY + DFT_ROOT_NOTE ) // Addition converts DFT index to MIDI key.

int16_t ai_raw_dft[ DFT_BOUND + 2 * DEWINDOW_BOUND + 1 ]; // Vector of raw DFT values.
int16_t* pai_raw_dft = ai_raw_dft;

int16_t ai_dft[ DFT_BOUND + 1 ]; // Vector of dewindowed DFT values.
int16_t* pai_dft = ai_dft;

int16_t ai_quorum[ DFT_BOUND + 1 ]; // Binary vector.  1 entries mark the smallest set of DFT values that make quorum.
int16_t* pai_quorum = ai_quorum;

int16_t ai_sieve[ DFT_BOUND + 1 ]; // Vector of estimates of notes played.
int16_t* pai_sieve = ai_sieve;

int16_t ai_note[ DFT_BOUND + 1 ]; // Vector of consolidated notes.  Each value is the sums of the values of its harmonics.
int16_t* pai_note = ai_note;

int16_t ai_channels[ CHANNEL_BOUND + 1 ]; // Vector of pointers to active DFT values.
int16_t* pai_channels = ai_channels;

int16_t ai_pitch_bend[ DFT_BOUND + 1 ]; // Vector of pitch-bend values.
int16_t* pai_pitch_bend = ai_pitch_bend;

state_t ae_state[ DFT_BOUND + 1 ]; // Vector of DFT value states.
state_t* pae_state = ae_state;

dft_config_t dft_config;
dewindow_config_t dewindow_config;
quorum_config_t quorum_config;
sieve_config_t sieve_config;
note_config_t note_config;
dynamic_config_t dynamic_config;
channel_config_t channel_config;
midi_config_t midi_config;

double d_dft_root_frequency; // The frequency of the first degree in the DFT.

int16_t i_audio_peak;
int16_t i_dft_peak;

#define FLANGESTY           us_midi_bank_msb = 88; us_midi_bank_lsb = 0;  us_midi_patch = 20;

mf_dft dft;
mf_dewindow dewindow;
mf_quorum quorum;
mf_sieve sieve;
mf_note note;
mf_dynamic dynamic;
mf_channel channel;
mf_midi midi;
mf_trigger trigger( SCOPE_TRIGGER );
mf_dac dac1( SCOPE_CH_1 );

void setup() {

  Serial.begin( 115200 );
  delay( 500 );

  d_dft_root_frequency = A2 * pow( 2.0, (double)( DFT_ROOT_NOTE ) / 12.0 );

uint8_t us_midi_bank_msb, us_midi_bank_lsb, us_midi_patch;

  /* MIDI voice */ FLANGESTY
  
  dft_config = {

    .i_pin_num_cs = ADC_CS,
    .i_pin_num_miso = ADC_MISO,
    .i_pin_num_clk = ADC_CLK,
    .d_dft_root_frequency = d_dft_root_frequency,
    .i_ref_cycle_bound = REF_CYCLES,
    .i_ref_datapoint_bound = REF_DATAPOINTS,
    .pi_audio_peak = &i_audio_peak,
    .pi_dft_peak = &i_dft_peak,
    .i_dft_bound = DFT_BOUND,
    .i_dewindow_bound = DEWINDOW_BOUND,
    .i_atten = DFT_ATTEN,
    .pae_state = pae_state,
    .i_bend_sensitivity = BEND_MESSAGE_BITS / BEND_RANGE,
    .d_pole = POLE,
    .pai_raw_dft = pai_raw_dft,
    .pai_pitch_bend = pai_pitch_bend

  };
  
  dewindow_config = {

    .i_dft_bound = DFT_BOUND,
    .i_dewindow_bound = DEWINDOW_BOUND,
    .pai_raw_dft = pai_raw_dft,
    .pai_dft = pai_dft
    
  };
  
  quorum_config = {

    .d_quorum = QUORUM_LIMIT,
    .i_threshold = QUORUM_THRESHOLD,
    .pi_dft_peak = &i_dft_peak,
    .i_input_bound = DFT_BOUND,    
    .pai_input = pai_dft,
    .i_output_bound = DFT_BOUND,
    .pai_output = pai_quorum

  };

  sieve_config = {
    
    .i_input_bound = DFT_BOUND,
    .pai_input = pai_quorum,
    .i_output_bound = DFT_BOUND,
    .pai_output = pai_sieve

  };
 
  note_config = {
    
    .i_input_bound = DFT_BOUND,
    .pai_input_1 = pai_sieve,
    .pai_input_2 = pai_dft,
    .i_output_bound = DFT_BOUND,
    .pai_output = pai_note

  };

  dynamic_config = {

    .i_settle_time_initial = SETTLE_TIME_INITIAL,
    .i_settle_time_gradient = SETTLE_TIME_GRADIENT,
    .i_input_bound = DFT_BOUND,
    .pai_input = pai_sieve,
    .i_output_bound = DFT_BOUND,
    .pae_output = pae_state

  };
 
  channel_config = {
    
    .i_input_1_bound = DFT_BOUND,
    .i_input_2_bound = CHANNEL_BOUND,
    .pae_input_1 = pae_state,
    .pai_input_2 = pai_channel

  };
  
  midi_config = {

    .us_CHANNEL_BASE = CHANNEL_BASE,
    .us_CHANNEL_BOUND = CHANNEL_BOUND,
    .us_midi_bank_msb = us_midi_bank_msb,
    .us_midi_bank_lsb = us_midi_bank_lsb,
    .us_midi_patch = us_midi_patch,
    .us_midi_out_pin = MIDI_OUT,
    .us_midi_in_pin = MIDI_IN,
    .i_key_from_degree = KEY_FROM_DEGREE,
    .i_bend_sensitivity = BEND_MESSAGE_BITS / BEND_RANGE,
    .i_input_bound = DFT_BOUND,
    .pai_channel = pai_channel,
    .pae_input_1 = pae_state,
    .pai_input_2 = pai_raw_dft,
    .pai_input_3 = pai_pitch_bend

  };

  dft.v_begin( &dft_config );
  dewindow.v_begin( &dewindow_config );
  quorum.v_begin( &quorum_config );
  sieve.v_begin( &sieve_config );
  note.v_begin( &note_config );
  dynamic.v_begin( &dynamic_config );
  channel.v_begin( &channel_config );
  midi.v_begin( &midi_config );
  
  for( int16_t i = 0; i <= DFT_BOUND; i++ ) {
    ae_state[ i ] = INACTIVE;
  }

}

void loop() {

  trigger.v_trigger();

  dft.v_raw_dft_from_audio(); // Create the raw DFT array from audio samples.

  dewindow.v_dewindow_baseband();

  quorum.v_quorum_from_dft();

  sieve.v_sieve_from_quorum();

  note.v_note_from_sieve();

  dynamic.v_state_from_seive();

  channel.v_allocate_channel();

  midi.v_midi_from_note();

}
