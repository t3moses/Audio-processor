
#ifndef SIEVE
  #define SIEVE

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

class mf_sieve {

  public:
  
    mf_sieve();
    void v_begin( sieve_config_t* p_sieve_config );
    void v_sieve_from_quorum();

  private:

    sieve_config_t* p_sieve_config;
    
    int16_t i_input_bound;
    int16_t*  pai_input;
    int16_t i_output_bound;
    int16_t* pai_output;
  
};

#endif
