#ifndef MIDI
  #define MIDI

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif


  class mf_midi {
    
    public:

      midi_config_t* p_midi_config;

      mf_midi();

      void v_begin( midi_config_t* p_midi_config );

      void v_midi_from_message();

    private:
      
      void note_on( int16_t i_channel, int16_t i_degree, int16_t i_velocity );

      void note_off( int16_t i_channel, int16_t i_degree );

      void note_bend( int16_t i_channel, int16_t i_bend );

      void all_off();

      midi_config_t* p_midi_config; 
      
      uint8_t us_midi_channel_base; // The MIDI channel number of the first channel.
      uint8_t us_midi_channel_bound; // Offset to the last channel
      uint8_t us_midi_bank_msb; // bank_msb, bank_lsb and patch define the sound generator voice.
      uint8_t us_midi_bank_lsb;
      uint8_t us_midi_patch;
      uint8_t us_midi_out_pin;
      uint8_t us_midi_in_pin;
      int16_t i_key_from_degree; // Conversion from degree to MIDI key number.
      int16_t i_bend_sensitivity; // End-to-end pitch-bend range ( 400 represents +/- 2 semitones ). 
      int16_t i_input_bound;
      int16_t* pai_channel; // Table for converting from channel to note.
      state_t* pae_input_1; // Pointer to the note state vector.
      int16_t* pai_input_2; // Pointer to the note data vector.
      int16_t* pai_input_3; // Pointer to the note pitch-bend vector.

  };
  
#endif
