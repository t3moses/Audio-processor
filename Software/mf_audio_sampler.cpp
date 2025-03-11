
#include "mf_audio_sampler.h"
#include "driver/timer.h"
#include "driver/gpio.h"

volatile int16_t _i_pin_num_cs;
volatile int16_t _i_pin_num_miso;
volatile int16_t _i_pin_num_clk;
volatile int16_t* __pi_audio_index;
volatile int16_t* __pi_audio;
volatile int16_t __i_audio_index_max;


spi_device_handle_t spi; // Analog-to-digital converter SPI.
spi_transaction_t adc_transaction_spec;

mf_audio_sampler::mf_audio_sampler() {}

void IRAM_ATTR timer_group0_isr( void *para ) {

//  volatile int timer_no = ( int )para;

  mf_audio_sampler::timer_group0_handler( /*timer_no*/ );

}


void mf_audio_sampler::v_begin( adc_pins_t* p_adc_pins, adc_config_t* p_adc_config ) {

// Convert adc parameters to global variables so that they become available to all class members.

  _i_pin_num_cs = p_adc_pins->i_pin_num_cs;
  _i_pin_num_miso = p_adc_pins->i_pin_num_miso;
  _i_pin_num_clk = p_adc_pins->i_pin_num_clk;

  __i_audio_index_max = p_adc_config->i_audio_index_max;
  __pi_audio_index = p_adc_config->pi_audio_index;
  __pi_audio = p_adc_config->pi_audio;
 
  mf_audio_sampler::v_set_up_adc();

}



void mf_audio_sampler::v_set_up_adc() {

// Set up the timer and spi interface.

esp_err_t return_code;
  
  spi_bus_config_t spi_configuration = {
    .mosi_io_num = -1,
    .miso_io_num = (gpio_num_t)_i_pin_num_miso,
    .sclk_io_num = (gpio_num_t)_i_pin_num_clk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = ADC_PRECISION
  };

  spi_device_interface_config_t adc_configuration = {
    .mode = 0,
    .clock_speed_hz = 10 * 1000 * 1000,
    .spics_io_num = (gpio_num_t)_i_pin_num_cs,
    .queue_size = 1,
    .pre_cb = mf_audio_sampler::adc_spi_pre_transfer_callback,
    .post_cb = mf_audio_sampler::adc_spi_post_transfer_callback,
  };

  return_code = spi_bus_initialize( ADC_HOST, &spi_configuration, DMA_CHAN );
  ESP_ERROR_CHECK( return_code );
  return_code = spi_bus_add_device( ADC_HOST, &adc_configuration, &spi );
  ESP_ERROR_CHECK( return_code );

  memset( &adc_transaction_spec, 0, sizeof( adc_transaction_spec ));
  adc_transaction_spec.rxlength = 16;
  adc_transaction_spec.length = 8 * 2;
  adc_transaction_spec.flags = SPI_TRANS_USE_RXDATA;

  // TIMER_GROUP_0, TIMER_0 controls the ADC.

  timer_config_t timer_0_config = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .intr_type = TIMER_INTR_LEVEL,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = SAMPLE_TIMER_DIVIDER
  };
  timer_init( TIMER_GROUP_0, TIMER_0, &timer_0_config );
  timer_set_counter_value( TIMER_GROUP_0, TIMER_0, 0x00000000ULL );
  timer_set_alarm_value( TIMER_GROUP_0, TIMER_0, TICKS_PER_SAMPLE );
  timer_enable_intr( TIMER_GROUP_0, TIMER_0 );
  timer_isr_register( TIMER_GROUP_0, TIMER_0, timer_group0_isr, (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL );
  timer_start( TIMER_GROUP_0, TIMER_0 );
}



void IRAM_ATTR mf_audio_sampler::timer_group0_handler( /*volatile int timer_number*/ ) {

// TIMER_0 interrupts populate pl_audio_frame with audio samples.
// TIMER_1 interrupts indicate that the audio level indicator flash period is over.

 esp_err_t return_code;
 timer_idx_t timer_no;

  timer_spinlock_take( TIMER_GROUP_0 );
//  timer_no = ( timer_idx_t )timer_number; // Which timer triggered the interrupt?

//  if ( timer_no == TIMER_0 ) {

    timer_group_clr_intr_status_in_isr( TIMER_GROUP_0, TIMER_0 );

    return_code = spi_device_polling_transmit( spi, &adc_transaction_spec );
    ESP_ERROR_CHECK( return_code );

    *__pi_audio_index = ( *__pi_audio_index < __i_audio_index_max ) ? *__pi_audio_index + 1 : *__pi_audio_index - __i_audio_index_max;

    *( __pi_audio + *__pi_audio_index ) = (((( (int16_t)(adc_transaction_spec.rx_data[0] )) << 6 ) + ( (int16_t)(adc_transaction_spec.rx_data[1] >> 2 ))) & 0x0FFF );

    //     MSB     |     LSB
    //     M    L  |   M    L
    //   xxdddddd  |   ddddddxx
    // .rx_data[0] | .rx_data[1]
    // ----------------------> t
    
    timer_group_enable_alarm_in_isr( TIMER_GROUP_0, TIMER_0 );

//  }

  timer_spinlock_give( TIMER_GROUP_0 );

}



void IRAM_ATTR mf_audio_sampler::adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec ) {

// Chip-select low fires the analog-to-digital converter sample-and-hold.

  gpio_set_level( (gpio_num_t)_i_pin_num_cs, 0);

}



void IRAM_ATTR mf_audio_sampler::adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec ) {

// Reset the analog-to-digital converter chip-select.

  gpio_set_level( (gpio_num_t)_i_pin_num_cs, 1);

}
