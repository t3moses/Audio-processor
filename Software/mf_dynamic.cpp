
#include "mf_types.h"
#include "mf_dynamic.h"



mf_dynamic::mf_dynamic() { }



void mf_dynamic::v_begin( dynamic_config_t* p_dynamic_config ) {

  mf_dynamic::p_dynamic_config = p_dynamic_config;

  mf_dynamic::i_settle_time_initial = mf_dynamic::p_dynamic_config->i_settle_time_initial;
  mf_dynamic::i_settle_time_gradient = mf_dynamic::p_dynamic_config->i_settle_time_gradient;
  mf_dynamic::i_input_bound = mf_dynamic::p_dynamic_config->i_input_bound;
  mf_dynamic::pai_input = mf_dynamic::p_dynamic_config->pai_input;
  mf_dynamic::i_output_bound = mf_dynamic::p_dynamic_config->i_output_bound;
  mf_dynamic::pae_output = mf_dynamic::p_dynamic_config->pae_output;

// Reset the output.

  for( int16_t i = 0; i <= mf_dynamic::i_output_bound; i++ ) {

    *( mf_dynamic::pae_output + i ) = INACTIVE;
    mf_dynamic::i_state_variable[ i ] = 0;
    
  }
}



void mf_dynamic::v_state_from_seive() {

  for( int16_t i = 0; i <= mf_dynamic::i_input_bound; i++ ) {

    switch( *( mf_dynamic::pae_output + i )) {

    case INACTIVE:

      if( *( mf_dynamic::pai_input + i ) == 1 ) {

      *( mf_dynamic::pae_output + i ) = PENDING_ACTIVE;
        mf_dynamic::i_state_variable[ i ] = i_settle_time_initial - i / i_settle_time_gradient;
          
      }
        
    break;

    case PENDING_ACTIVE:

      if( *( mf_dynamic::pai_input + i ) != 1 ) {

        *( mf_dynamic::pae_output + i ) = INACTIVE;
        mf_dynamic::i_state_variable[ i ] = 0;
        
      }

      else {

        mf_dynamic::i_state_variable[ i ]--;
        
        if( mf_dynamic::i_state_variable[ i ] <= 0 ) {

          *( mf_dynamic::pae_output + i ) = ATTACK;
          mf_dynamic::i_state_variable[ i ] = 0;
 
        }

      }     

    break;

    case ATTACK:

      if( *( mf_dynamic::pai_input + i ) != 1 ) {

        *( mf_dynamic::pae_output + i ) = RELEASE;
        mf_dynamic::i_state_variable[ i ] = 0;
        
      }
      
      else {
              
        *( mf_dynamic::pae_output + i ) = SUSTAIN;
        mf_dynamic::i_state_variable[ i ] = 0;
      
      }
            
    break;

    case SUSTAIN:

      if( *( mf_dynamic::pai_input + i ) != 1 ) {

        *( mf_dynamic::pae_output + i ) = RELEASE;
        mf_dynamic::i_state_variable[ i ] = 0;

      }

    break;

    case RELEASE:
      
      *( mf_dynamic::pae_output + i ) = INACTIVE;
      mf_dynamic::i_state_variable[ i ] = 0;
      
    break;

    }
  }
}
