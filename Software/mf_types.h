#ifndef TYPES
  #define TYPES

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

enum state_t { INACTIVE, PENDING_ACTIVE, ATTACK, SUSTAIN, RELEASE };

struct dft_config_t {
  
  int16_t  i_pin_num_cs; // ADC chip select pin.
  int16_t  i_pin_num_miso; // ADC MISO pin.
  int16_t  i_pin_num_clk; // ADC clock pin.
  double d_dft_root_frequency; // Frequency of the note corresponding to model degree 0.
  int16_t i_ref_cycle_bound;
  int16_t i_ref_datapoint_bound;
  int16_t* pi_audio_peak;
  int16_t* pi_dft_peak;
  int16_t i_dft_bound;
  int16_t i_dewindow_bound;
  int16_t i_atten;
  state_t* pae_state;
  int16_t i_bend_sensitivity;
  double d_pole;
  int16_t* pai_raw_dft; // Pointer to the DFT output.
  double* pad_pitch_bend; // Pointer to the pitch-bend output.

};

struct dewindow_config_t { // Configuration data for the dewindow function.

  int16_t i_dft_bound;
  int16_t i_dewindow_bound;
  int16_t* pai_input; // Pointer to the dewindow input.
  int16_t* pai_output;
  
};

struct quorum_config_t { // Configuration data for the quorum function.

  double d_quorum;
  int16_t i_threshold;
  int16_t* pi_dft_peak;
  int16_t i_input_bound;
  int16_t* pai_input;
  int16_t i_output_bound;
  int16_t* pai_output;

};

struct sieve_config_t { // Configuration data for the sieve function.

  int16_t i_input_bound;
  int16_t*  pai_input;
  int16_t i_output_bound;
  int16_t* pai_output;

};

struct note_config_t { // Configuration data for the note function.

  int16_t i_input_bound;
  int16_t*  pai_input_1;
  int16_t*  pai_input_2;
  int16_t i_output_bound;
  int16_t* pai_output;

};

struct dynamic_config_t { // Configuration data for the dynamic function.

  int16_t i_settle_time_initial;
  int16_t i_settle_time_gradient;
  int16_t i_input_bound;
  int16_t* pai_input;
  int16_t i_output_bound;
  state_t* pae_output;
  
};

struct channel_config_t { // Configuration data for the channel function.

  int16_t i_input_1_bound;
  int16_t i_input_2_bound;
  state_t* pae_input_1; // Pointer to the degree state vector.
  int16_t* pai_input_2; // Pointer to the channel allocation array.

};

struct midi_config_t { // Configuration data for the MIDI function.

  uint8_t us_midi_channel_base; // The MIDI channel number of the first channel.
  uint8_t us_midi_channel_bound; // Offset to the last channel
  uint8_t us_midi_bank_msb; // bank_msb, bank_lsb and patch define the sound generator voice.
  uint8_t us_midi_bank_lsb;
  uint8_t us_midi_patch;
  uint8_t us_midi_out_pin;
  uint8_t us_midi_in_pin;
  int16_t i_key_from_degree; // Conversion from degree to MIDI key number.
  int16_t i_bend_sensitivity;
  int16_t i_input_bound;
  int16_t* pai_channel; // Conversion from channel to note.
  state_t* pae_input_1; // Pointer to the note state vector.
  int16_t* pai_input_2; // Pointer to the note data vector.
  int16_t* pai_input_3; // Pointer to the note pitch-bend vector.

};

#endif
