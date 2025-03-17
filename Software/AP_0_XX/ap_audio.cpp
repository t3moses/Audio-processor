
#include "ap_constants.h"
#include "ap_types.h"
#include "ap_audio.h"
#include "driver/timer.h"
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

void IRAM_ATTR timer_group0_isr( void *para ) {

  volatile int timer_no = ( int )para;

  ap_audio::timer_group0_handler( timer_no );

}

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
  ap_audio::v_set_up_adc();
  ap_audio::v_set_up_dac();
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
    .max_transfer_sz = ADC_PRECISION

  };

  return_code = spi_bus_initialize( SPI_HOST, &spi_config, DMA_CHAN );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

}



void ap_audio::v_set_up_adc() {

esp_err_t return_code;

// Configuration for the left channel ADC

  spi_device_interface_config_t left_adc_spi_configuration = {

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

// Configuration for the right channel ADC

  spi_device_interface_config_t right_adc_spi_configuration = {

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

  return_code = spi_bus_add_device( SPI_HOST, &left_adc_spi_configuration, &left_adc_spi_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().
  return_code = spi_bus_add_device( SPI_HOST, &right_adc_spi_configuration, &right_adc_spi_handle );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

  memset( &adc_transaction_spec, 0, sizeof( adc_transaction_spec ));
  adc_transaction_spec.rxlength = 16;
  adc_transaction_spec.length = 16;
  adc_transaction_spec.flags = SPI_TRANS_USE_RXDATA;

  return_code = spi_device_polling_transmit( left_adc_spi_handle, &adc_transaction_spec );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().
  return_code = spi_device_polling_transmit( right_adc_spi_handle, &adc_transaction_spec );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message on the console, and call abort().

}



void ap_audio::v_set_up_dac() {

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
  
  gpio_set_level( (gpio_num_t)_i_pin_num_dac_ldac, 0);

}



void ap_audio::v_setup_timers() {

// TIMER_GROUP_0, TIMER_0 controls the left and right ADCs.
// TIMER_GROUP_0, TIMER_1 controls the DAC.

  timer_config_t timer_0_config = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .intr_type = TIMER_INTR_LEVEL,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = SAMPLE_TIMER_DIVIDER
  };

  timer_config_t timer_1_config = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .intr_type = TIMER_INTR_LEVEL,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = SAMPLE_TIMER_DIVIDER
  };

  noInterrupts();

  timer_init( TIMER_GROUP_0, TIMER_0, &timer_0_config );
  timer_set_counter_value( TIMER_GROUP_0, TIMER_0, 0x00000000ULL );
  timer_set_alarm_value( TIMER_GROUP_0, TIMER_0, TICKS_PER_AUDIO_IN_SAMPLE );
  timer_enable_intr( TIMER_GROUP_0, TIMER_0 );
  timer_isr_register( TIMER_GROUP_0, TIMER_0, timer_group0_isr, (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL );
  timer_start( TIMER_GROUP_0, TIMER_0 );

  timer_init( TIMER_GROUP_0, TIMER_1, &timer_1_config );
  timer_set_counter_value( TIMER_GROUP_0, TIMER_1, 0x00000000ULL );
  timer_set_alarm_value( TIMER_GROUP_0, TIMER_1, TICKS_PER_AUDIO_OUT_SAMPLE );
  timer_enable_intr( TIMER_GROUP_0, TIMER_1 );
  timer_isr_register( TIMER_GROUP_0, TIMER_1, timer_group0_isr, (void *)TIMER_1, ESP_INTR_FLAG_IRAM, NULL );
  timer_start( TIMER_GROUP_0, TIMER_1 );

  interrupts();

}



void IRAM_ATTR ap_audio::timer_group0_handler( volatile int timer_number ) {

 timer_idx_t timer_no;

  timer_spinlock_take( TIMER_GROUP_0 );
  timer_no = ( timer_idx_t )timer_number; // Which timer triggered the interrupt?

  if ( timer_no == TIMER_0 ) { // The interrupt was triggered by TIMER_0.  So, process the left and right ADCs.

    ap_audio::procees_adc();

  }

  else { // The interrupt was triggered by TIMER_1.  So, process the DAC.

    ap_audio::procees_dac();

  }

  timer_spinlock_give( TIMER_GROUP_0 );

}



void IRAM_ATTR ap_audio::procees_adc() {

esp_err_t return_code;

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

  timer_group_enable_alarm_in_isr( TIMER_GROUP_0, TIMER_0 );
  timer_group_clr_intr_status_in_isr( TIMER_GROUP_0, TIMER_0 );

}



void IRAM_ATTR ap_audio::procees_dac() {

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

      .length = 16,      // 16 bits
      .tx_buffer = &left_data,
      .rx_buffer = NULL  // We're not reading anything

  };

  spi_transaction_t right_update = {

      .length = 16,      // 16 bits
      .tx_buffer = &right_data,
      .rx_buffer = NULL  // We're not reading anything

  };

  return_code = spi_device_transmit(dac_spi_handle, &left_update);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

  return_code = spi_device_transmit(dac_spi_handle, &right_update);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then an error message is printed on the console, and abort() is called.

  timer_group_enable_alarm_in_isr( TIMER_GROUP_0, TIMER_1 );
  timer_group_clr_intr_status_in_isr( TIMER_GROUP_0, TIMER_1 );

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
