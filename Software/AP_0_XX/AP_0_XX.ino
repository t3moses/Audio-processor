
#include "ap_constants.h"
#include "ap_types.h"
#include "ap_audio.h"

ap_audio audio; // Instantiate an audio object.

audio_sample_t xa_audio_in[ AUDIO_IN_ARRAY_INDEX_MAX + 1 ]; // Array of audio-in samples.
audio_sample_t* pxa_audio_in = xa_audio_in; // Pointer to the array of audio-in samples.

int16_t i_audio_index = 0; // Index of the most recent audio-in sample.
int16_t* pi_audio_index = &i_audio_index; // Pointer to the index of the most recent audio-in sample.

audio_sample_t x_audio_out; // Audio out sample.
audio_sample_t* px_audio_out = &x_audio_out; // Pointer to audio out sample.

void setup() {

  Serial.begin( 115200 );
  delay( 500 );

  spi_config_t spi_config = {

    .i_pin_num_in_left_cs = ADC_LEFT_CS, // Left channel ADC chip select pin.
    .i_pin_num_in_right_cs = ADC_RIGHT_CS, // Right channel ADC chip select pin.
    .i_pin_num_out_cs = DAC_CS, // DAC chip select pin.
    .i_pin_num_out_ldac = DAC_LDAC, // DAC LDAC pin.
    .i_pin_num_miso = ADC_MISO, // ADC MISO pin.
    .i_pin_num_mosi = DAC_MOSI, // DAC MOSI pin.
    .i_pin_num_clk = SPI_CLK, // ADC clock pin.

  };

  adc_config_t adc_config = {

    .i_audio_index_max = AUDIO_IN_ARRAY_INDEX_MAX, // Maximm value of the index to the audio sample array.
    .pi_audio_index = pi_audio_index, // Pointer to the index of the most recent audio sample.
    .pxa_audio_samples = pxa_audio_in // Pointer to an array of audio samples.

  };

  dac_config_t dac_config = {

    .px_audio_sample = px_audio_out // Pointer to an audio sample.

  };

  audio.v_begin( &spi_config, &adc_config, &dac_config );

}

void loop() {



}
