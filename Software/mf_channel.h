#ifndef CHANNEL
  #define CHANNEL

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif


  class mf_channel {
    
    public:

      mf_channel();

      void v_begin( channel_config_t* p_channel_config );

      void v_allocate_channel();

    private:

      channel_config_t* p_channel_config;

      int16_t i_input_1_bound;
      int16_t i_input_2_bound;
      state_t* pae_input_1;
      int16_t* pai_input_2;

  };
  
#endif
