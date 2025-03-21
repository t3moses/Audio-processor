#include "sdkconfig.h"
#include "ap_constants.h"
#include "ap_types.h"
#include "ap_audio.h"
#include "esp_timer.h"
#include "driver/gpio.h"

volatile int16_t _i_pin_num_left_adc_cs; // Class-scope variables.
volatile int16_t _i_pin_num_right_adc_cs;
volatile int16_t _i_pin_num_dac_cs;
volatile int16_t _i_pin_num_dac_ldac;
volatile int16_t _i_pin_num_miso;
volatile int16_t _i_pin_num_mosi;
volatile int16_t _i_pin_num_clk;
volatile int16_t _i_audio_in_index_max;
volatile int16_t* _pi_audio_in_index;
audio_sample_t* _pxa_audio_samples;
audio_sample_t* _px_audio_sample;

spi_device_handle_t left_adc_spi_handle;
spi_device_handle_t right_adc_spi_handle;
spi_device_handle_t dac_spi_handle;
spi_transaction_t adc_transaction_spec;
spi_transaction_t dac_transaction_spec;

audio_sample_t sample;

ap_audio::ap_audio() {}



void ap_audio::v_begin( spi_config_t* p_spi_config, adc_config_t* p_adc_config, dac_config_t* p_dac_config ) {

// Convert spi parameters to class-global variables so that they become available to all class members.

  _i_pin_num_left_adc_cs = p_spi_config->i_pin_num_in_left_cs;
  _i_pin_num_right_adc_cs = p_spi_config->i_pin_num_in_right_cs;
  _i_pin_num_dac_cs = p_spi_config->i_pin_num_out_cs;
  _i_pin_num_dac_ldac = p_spi_config->i_pin_num_out_ldac;
  _i_pin_num_miso = p_spi_config->i_pin_num_miso;
  _i_pin_num_mosi = p_spi_config->i_pin_num_mosi;
  _i_pin_num_clk = p_spi_config->i_pin_num_clk;

// Convert adc parameters to class-global variables.

  _i_audio_in_index_max = p_adc_config->i_audio_index_max;
  _pi_audio_in_index = p_adc_config->pi_audio_index;
  _pxa_audio_samples = p_adc_config->pxa_audio_samples;

// Convert dac parameters to class-global variables.

  _px_audio_sample = p_dac_config->px_audio_sample;
 
  ap_audio::v_setup_spi();
  ap_audio::v_setup_adc();
  ap_audio::v_setup_dac();
  ap_audio::v_setup_timers();

}



void ap_audio::v_setup_spi() {

esp_err_t return_code;

  spi_bus_config_t spi_config = {

    .mosi_io_num = (gpio_num_t)_i_pin_num_mosi,
    .miso_io_num = (gpio_num_t)_i_pin_num_miso,
    .sclk_io_num = (gpio_num_t)_i_pin_num_clk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 32,

  };

  return_code = spi_bus_initialize( SPI_HOST, &spi_config, DMA_CHAN );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

}



void ap_audio::v_setup_adc() {

esp_err_t return_code;

// config for the left channel ADC

  spi_device_interface_config_t left_adc_spi_config = {

    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .clock_speed_hz = 10 * 1000 * 1000,
    .spics_io_num = (gpio_num_t)_i_pin_num_left_adc_cs,
    .queue_size = 1,
    .pre_cb = ap_audio::left_adc_spi_pre_transfer_callback,
    .post_cb = ap_audio::left_adc_spi_post_transfer_callback,

  };

// config for the right channel ADC

  spi_device_interface_config_t right_adc_spi_config = {

    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .clock_speed_hz = 10 * 1000 * 1000,
    .spics_io_num = (gpio_num_t)_i_pin_num_right_adc_cs,
    .queue_size = 1,
    .pre_cb = ap_audio::right_adc_spi_pre_transfer_callback,
    .post_cb = ap_audio::right_adc_spi_post_transfer_callback,

  };

// Add the ADCs to the bus.

  return_code = spi_bus_add_device( SPI_HOST, &left_adc_spi_config, &left_adc_spi_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().
  return_code = spi_bus_add_device( SPI_HOST, &right_adc_spi_config, &right_adc_spi_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

// Set up the ADC SPI transaction spec and send it to the SPI host.

  memset( &adc_transaction_spec, 0, sizeof( adc_transaction_spec ));
  adc_transaction_spec.rxlength = 16;
  adc_transaction_spec.length = 16;
  adc_transaction_spec.flags = SPI_TRANS_USE_RXDATA;

  return_code = spi_device_polling_transmit( left_adc_spi_handle, &adc_transaction_spec );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().
  return_code = spi_device_polling_transmit( right_adc_spi_handle, &adc_transaction_spec );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

}



void ap_audio::v_setup_dac() {

esp_err_t return_code;

// Configuration for the DAC

  spi_device_interface_config_t dac_config = {

    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .clock_speed_hz = 10 * 1000 * 1000,
    .spics_io_num = (gpio_num_t)_i_pin_num_dac_cs,
    .queue_size = 1,
    .pre_cb = ap_audio::dac_spi_pre_transfer_callback,
    .post_cb = ap_audio::dac_spi_post_transfer_callback,

  };

// Add the DAC to the bus.

  return_code = spi_bus_add_device(SPI_HOST, &dac_config, &dac_spi_handle);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

  memset( &dac_transaction_spec, 0, sizeof( dac_transaction_spec ));
  dac_transaction_spec.rxlength = 16;
  dac_transaction_spec.length = 16;
  dac_transaction_spec.flags = SPI_TRANS_USE_TXDATA;

  return_code = spi_device_polling_transmit( dac_spi_handle, &dac_transaction_spec );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

// Automatically transfer DAC input register contents to the corresponding output registers.

  gpio_set_level( (gpio_num_t)_i_pin_num_dac_ldac, 0 );

}



void ap_audio::v_setup_timers() {

esp_err_t return_code;

// Set up timers for the ADC and DAC.

esp_timer_handle_t adc_timer_handle;
esp_timer_handle_t dac_timer_handle;

uint64_t u64_adc_timer_interval_us = 10 * 1000 * 1000 / AUDIO_IN_SAMPLES_PER_SECOND;
uint64_t u64_dac_timer_interval_us = 10 * 1000 * 1000 / AUDIO_OUT_SAMPLES_PER_SECOND;

  const esp_timer_create_args_t adc_timer_config = {
    .callback = &ap_audio::process_adc,
    .dispatch_method = ESP_TIMER_ISR,
    .name = "adc_timer",
  };

  const esp_timer_create_args_t dac_timer_config = {
    .callback = &ap_audio::process_dac,
    .dispatch_method = ESP_TIMER_ISR,
    .name = "dac_timer",
  };

  return_code = esp_timer_create( &adc_timer_config, &adc_timer_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.
  return_code = esp_timer_stop( adc_timer_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.
  return_code = esp_timer_start_periodic( adc_timer_handle, u64_adc_timer_interval_us );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

  return_code = esp_timer_create( &dac_timer_config, &dac_timer_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.
  return_code = esp_timer_stop( dac_timer_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.
  return_code = esp_timer_start_periodic( dac_timer_handle, u64_dac_timer_interval_us );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

}

void IRAM_ATTR ap_audio::process_adc(void* arg) {

esp_err_t return_code;
audio_sample_t sample;

  // Read the left and right channel inputs.
  //
  //     MSB     |     LSB
  //     M    L  |   M    L
  //   xxdddddd  |   ddddddxx
  // .rx_data[0] | .rx_data[1]
  // ----------------------> t

  sample.left = (((( (int16_t)(adc_transaction_spec.rx_data[0] )) << 6 ) + ( (int16_t)(adc_transaction_spec.rx_data[1] >> 2 ))) & 0x0FFF );
  sample.right = (((( (int16_t)(adc_transaction_spec.rx_data[0] )) << 6 ) + ( (int16_t)(adc_transaction_spec.rx_data[1] >> 2 ))) & 0x0FFF );;

  // Update the audio buffer pointer and write audio data to the buffer.

  *_pi_audio_in_index = ( *_pi_audio_in_index < _i_audio_in_index_max ) ? *_pi_audio_in_index + 1 : *_pi_audio_in_index - _i_audio_in_index_max;
  *( _pxa_audio_samples + *_pi_audio_in_index ) = sample;

}



void IRAM_ATTR ap_audio::process_dac(void* arg) {

esp_err_t return_code;
uint16_t left_data;
uint16_t right_data;

  // Prepare the left channel output.

  left_data = 0;
  left_data |= (LEFT & 0x01) << 15;               // Channel select bit (A=0, B=1) in bit 15
  left_data |= (DAC_GAIN & 0x01) << 13;           // Gain select bit (1x=1, 2x=0) in bit 13
  left_data |= (1 << 12);                         // SHDN (shutdown bit) 1 = active
  left_data |= (_px_audio_sample->left & 0x0FFF); // 12-bit DAC value in bits 0-11

  right_data = 0;
  right_data |= (RIGHT & 0x01) << 15;               // Channel select bit (A=0, B=1) in bit 15
  right_data |= (DAC_GAIN & 0x01) << 13;            // Gain select bit (1x=1, 2x=0) in bit 13
  right_data |= (1 << 12);                          // SHDN (shutdown bit) 1 = active
  right_data |= (_px_audio_sample->right & 0x0FFF); // 12-bit DAC value in bits 0-11
  
  // Convert to big-endian (network byte order) as required by MCP4822

  left_data = SPI_SWAP_DATA_TX(left_data, 16);
  right_data = SPI_SWAP_DATA_TX(right_data, 16);
  
  // Create the left and right channel updates.

  spi_transaction_t left_update = {

      .length = 16,
      .tx_buffer = &left_data,
      .rx_buffer = NULL

  };

  spi_transaction_t right_update = {

      .length = 16,
      .tx_buffer = &right_data,
      .rx_buffer = NULL

  };

  return_code = spi_device_transmit(dac_spi_handle, &left_update);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

  return_code = spi_device_transmit(dac_spi_handle, &right_update);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

}



// Toggle the appropriate chip select pin.

void IRAM_ATTR ap_audio::left_adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_left_adc_cs, 0);
}

void IRAM_ATTR ap_audio::left_adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_left_adc_cs, 1);
}

void IRAM_ATTR ap_audio::right_adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_right_adc_cs, 0);
}

void IRAM_ATTR ap_audio::right_adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_right_adc_cs, 1);
}

void IRAM_ATTR ap_audio::dac_spi_pre_transfer_callback( spi_transaction_t* dac_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_dac_cs, 0);
}

void IRAM_ATTR ap_audio::dac_spi_post_transfer_callback( spi_transaction_t* dac_transaction_spec ) {
  gpio_set_level( (gpio_num_t)_i_pin_num_dac_cs, 1);
}
