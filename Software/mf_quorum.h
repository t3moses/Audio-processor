
#ifndef QUORUM
  #define QUORUM

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

#define MODEL_BOUND  36

class mf_quorum {

  public:

  mf_quorum();
  void v_begin( quorum_config_t* p_quorum_config );
  void v_quorum_from_dft();
  
private:
  
  quorum_config_t* p_quorum_config;

  double d_quorum_level;
  int16_t i_quorum_threshold;
  int16_t* pi_dft_peak;
  int16_t i_input_bound;
  int16_t* pai_input;
  int16_t i_output_bound;
  int16_t*  pai_output;
  
};

#endif  
