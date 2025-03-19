
#ifndef AUDIO
  #define AUDIO

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  #ifndef SPI
    #define SPI
    #include "driver/spi_master.h"
  #endif

  class ap_audio {
    
    public:
    
      ap_audio();
      
      void v_begin( spi_config_t* p_spi_config, adc_config_t* p_adc_config, dac_config_t* p_dac_config );

      void v_setup_spi();

      void v_setup_adc();

      void v_setup_dac();

      void v_setup_timers();

      static void IRAM_ATTR process_adc(void* arg);

      static void IRAM_ATTR process_dac(void* arg);

      static void IRAM_ATTR left_adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec );

      static void IRAM_ATTR left_adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec );
    
      static void IRAM_ATTR right_adc_spi_pre_transfer_callback( spi_transaction_t* adc_transaction_spec );

      static void IRAM_ATTR right_adc_spi_post_transfer_callback( spi_transaction_t* adc_transaction_spec );
 
      static void IRAM_ATTR dac_spi_pre_transfer_callback( spi_transaction_t* dac_transaction_spec );

      static void IRAM_ATTR dac_spi_post_transfer_callback( spi_transaction_t* dac_transaction_spec );
    
    private:

      static void IRAM_ATTR procees_adc();

      static void IRAM_ATTR procees_dac();
    
  };
  
#endif
