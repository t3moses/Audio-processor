
#include "mf_types.h"
#include "mf_note.h"

mf_note::mf_note() { }

void mf_note::v_begin( note_config_t* p_note_config ) {

  mf_note::p_note_config = p_note_config;
    
  mf_note::i_input_bound = mf_note::p_note_config->i_input_bound;
  mf_note::pai_input_1 = mf_note::p_note_config->pai_input_1;
  mf_note::pai_input_2 = mf_note::p_note_config->pai_input_2;
  mf_note::i_output_bound = mf_note::p_note_config->i_output_bound;
  mf_note::pai_output = mf_note::p_note_config->pai_output;

}

void mf_note::v_note_from_sieve() {

// Parse pai_input_1 looking for entries == 1.
// For each, add all the energies from its harmonics (12, 19, 24, 28, 36).

int16_t j;

  for( int16_t i = 0; i <= mf_note::i_input_bound; i++ ) {

    if( *( mf_note::pai_input_1 + i ) == 1 ) {

      *( mf_note::pai_output + i ) = *( mf_note::pai_input_2 + i );
      j = 12;
      if(( i + j ) <= mf_note::i_input_bound ) *( mf_note::pai_output + i ) += *( mf_note::pai_input_2 + i + j );
      j = 19;
      if(( i + j ) <= mf_note::i_input_bound ) *( mf_note::pai_output + i ) += *( mf_note::pai_input_2 + i + j );
      j = 24;
      if(( i + j ) <= mf_note::i_input_bound ) *( mf_note::pai_output + i ) += *( mf_note::pai_input_2 + i + j );
      j = 28;
      if(( i + j ) <= mf_note::i_input_bound ) *( mf_note::pai_output + i ) += *( mf_note::pai_input_2 + i + j );
      j = 36;
      if(( i + j ) <= mf_note::i_input_bound ) *( mf_note::pai_output + i ) += *( mf_note::pai_input_2 + i + j );

    }

    else *( mf_note::pai_output + i ) = 0;
    
  }
  
}
