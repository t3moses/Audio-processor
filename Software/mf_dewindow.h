
#ifndef DEWINDOW
  #define DEWINDOW

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

class mf_dewindow {

  public:
  
    mf_dewindow();
    void v_begin( dewindow_config_t* p_dewindow_config );
    void v_dewindow_baseband();
  
  private:

    dewindow_config_t* p_dewindow_config;

    int16_t i_raw_dft_bound;
    int16_t i_dewindow_bound;
    int16_t* pai_raw_dft;
    int16_t* pai_dft;

    int16_t i_dft_bound;
 
};

#endif
