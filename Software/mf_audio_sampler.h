
#ifndef SAMPLER
  #define SAMPLER

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  #ifndef SPI
    #define SPI
    #include "driver/spi_master.h"
  #endif

#define ADC_HOST                          SPI2_HOST  // Identifies the HSPI controller.
#define DMA_CHAN                          0          // DMA not being used.
#define ADC_PRECISION                     2          // The number of bytes per data point.

#define SAMPLES_PER_SECOND                25000
#define SAMPLE_TIMER_DIVIDER              16 // Hardware timer clock divider for sample timer.
#define SAMPLE_TICKS_PER_SECOND           (TIMER_BASE_CLK / SAMPLE_TIMER_DIVIDER)
#define TICKS_PER_SAMPLE                  SAMPLE_TICKS_PER_SECOND / SAMPLES_PER_SECOND

struct adc_config_t {
  
  int16_t i_audio_index_max;
  int16_t* pi_audio_index;
  int16_t* pi_audio;

};

struct adc_pins_t {
  
  int16_t  i_pin_num_cs;
  int16_t  i_pin_num_miso;
  int16_t  i_pin_num_clk;

};

  class mf_audio_sampler {
    
    public:
    
      mf_audio_sampler();
      
      void v_begin( adc_pins_t* p_adc_pins, adc_config_t* p_adc_config );

      void v_set_up_adc();

      static void IRAM_ATTR timer_group0_handler( /*volatile int timer_number*/ );
    
      static void IRAM_ATTR adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec );

      static void IRAM_ATTR adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec );

  };
  
#endif
