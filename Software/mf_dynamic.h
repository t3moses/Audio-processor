
#ifndef DYNAMIC
  #define DYNAMIC

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

#define STATE_BOUND 49

class mf_dynamic {

  public:

  mf_dynamic();
  void v_begin( dynamic_config_t* p_dynamic_config );
  void v_state_from_seive();
  
private:
  
  dynamic_config_t* p_dynamic_config;
  int16_t i_state_variable[ STATE_BOUND + 1 ];

  int16_t i_settle_time_initial;
  int16_t i_settle_time_gradient;
  int16_t i_input_bound;
  int16_t* pai_input;
  int16_t i_output_bound;
  state_t* pae_output;
  
};

#endif  
