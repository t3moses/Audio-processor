#ifndef TYPES
  #define TYPES

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

struct audio_sample_t {

  int16_t left;
  int16_t right;

};

struct spi_config_t {
  
  int16_t         i_pin_num_in_left_cs; // Left channel ADC chip select pin.
  int16_t         i_pin_num_in_right_cs; // Right channel ADC chip select pin.
  int16_t         i_pin_num_out_cs; // DAC chip select pin.
  int16_t         i_pin_num_out_ldac; // DAC load pin.
  int16_t         i_pin_num_miso; // ADC MISO pin.
  int16_t         i_pin_num_mosi; // DAC MOSI pin.
  int16_t         i_pin_num_clk; // ADC clock pin.

};

struct adc_config_t {
  
  int16_t         i_audio_index_max; // Maximm value of the index to the audio sample array.
  int16_t*        pi_audio_index; // Pointer to the index of the most recent audio sample.
  audio_sample_t* pxa_audio_samples; // Pointer to an array of audio samples.

};

struct dac_config_t {
  
  audio_sample_t* px_audio_sample; // Pointer to an audio sample.

};

#endif
