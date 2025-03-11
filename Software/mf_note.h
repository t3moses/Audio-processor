
#ifndef NOTE
  #define NOTE

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

class mf_note {

  public:

  mf_note();
  void v_begin( note_config_t* p_note_config );
  void v_note_from_sieve();
  
private:
  
  note_config_t* p_note_config;

  int16_t i_input_bound;
  int16_t* pai_input_1;
  int16_t* pai_input_2;
  int16_t i_output_bound;
  int16_t* pai_output;
  
};

#endif  
