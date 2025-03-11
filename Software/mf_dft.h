#ifndef DFT
  #define DFT

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif


#define AUDIO_BOUND 5000
#define REF_CYCLE_BOUND 15
#define REF_DATAPOINT_BOUND 29
#define NOTE_BOUND 49
#define LSB_PER_VOLT ( 4095.0 / 3.2 )
#define REFERENCE_AMPLITUDE 1.5
#define ADC_OFFSET_SAMPLE_BOUND 999 // ADC offset calculated over audio samples 0..1023.
#define ADC_BITS 12


class mf_dft {

  public:
  
    mf_dft();
    void v_begin( dft_config_t* p_dft_config );
    void v_raw_dft_from_audio();

  private:

    dft_config_t* p_dft_config;
  
    int16_t i_pin_num_cs;
    int16_t i_pin_num_miso;
    int16_t i_pin_num_clk;
    double d_dft_root_frequency;
    int16_t i_ref_cycle_bound;
    int16_t i_ref_datapoint_bound;
    int16_t* pi_audio_peak;
    int16_t* pi_dft_peak;
    int16_t i_model_bound;
    int16_t i_dewindow_bound;
    int16_t i_atten;
    state_t* pae_state;
    int16_t i_bend_sensitivity;
    double d_pole;
    int16_t* pai_raw_dft;
    double* pad_pitch_bend;
    
    int32_t l_scale_factor;

    int16_t i_audio_index;
    int16_t* pi_audio_index = &i_audio_index;
    int16_t ai_audio[ AUDIO_BOUND + 1 ]; // Array of audio samples.
    int16_t* pai_audio = ai_audio; // Pointer to the array of audio samples.

    double d_degree_factor;
    double d_raw_dft_root_frequency;
    double d_ref_frequency;
    double d_loop_forward_path_constant; // Forward loop constant.
    double d_loop_reverse_path_constant; // Reverse loop constant.
    double d_subsample_interval; // The number of audio samples between adjacent time-domain bins in the DFT calculation.
    double d_ideal_sample_time;
    double ad_pitch_correction[ NOTE_BOUND + 1 ];
    double* pad_pitch_correction = ad_pitch_correction;
    double d_pitch_offset;
    double ad_cycle_phase[ REF_CYCLE_BOUND + 1 ];
    double* pad_cycle_phase = ad_cycle_phase;
     
    int32_t al_cycle_inphase[ REF_CYCLE_BOUND + 1 ];
    int32_t* pal_cycle_inphase = al_cycle_inphase;
    int32_t al_cycle_quadrature[ REF_CYCLE_BOUND + 1 ];
    int32_t* pal_cycle_quadrature = al_cycle_quadrature;
    int32_t l_cycle_inphase_component;
    int32_t l_cycle_quadrature_component;
    int32_t l_degree_inphase_component;
    int32_t l_degree_quadrature_component;

    int16_t i_note;
    int16_t i_ref_cycle;
    int16_t i_ref_datapoint;
    int16_t i_ref_inphase_value[ REF_DATAPOINT_BOUND + 1 ];
    int16_t i_ref_quadrature_value[ REF_DATAPOINT_BOUND + 1 ];
    int16_t i_adc_offset;
    int16_t i_audio_sample_value;
    int16_t i_best_sample;
    int16_t i_read_index_after;
    int16_t i_read_index_before;
    int16_t i_write_index_copy;
    int16_t i_raw_sample_before;
    int16_t i_raw_sample_after;

// Functions

    void v_set_up_reference_values();
    void v_measure_adc_offset();
    void v_sample_from_audio();
    void v_cycle_phase_from_components();
    void v_note_pitch_offset_from_cycle_phase_array();
  
};

#endif
