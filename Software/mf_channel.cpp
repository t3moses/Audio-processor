
#include "mf_types.h"
#include <mf_channel.h>



mf_channel::mf_channel() {}



void mf_channel::v_begin( channel_config_t* p_channel_config ) {

  mf_channel::p_channel_config = p_channel_config;

  mf_channel::i_input_1_bound = mf_channel::p_channel_config->i_input_1_bound;
  mf_channel::i_input_2_bound = mf_channel::p_channel_config->i_input_2_bound;
  mf_channel::pae_input_1 = mf_channel::p_channel_config->pae_input_1;
  mf_channel::pai_input_2 = mf_channel::p_channel_config->pai_input_2;

}



void mf_channel::v_allocate_channel() {

// Go through the array of DFT states.  If an ATTACK state is found, then allocate an unused channel to that DFT value.
// Go through the channel allocation array until an array entry points to an INACTIVE DFT value.
// Then allocate that channel to the DFT value.  If no INACTIVE DFT value is found, then do nothing.

  for( int16_t i = 0; i <= mf_channel::i_input_1_bound; i++ ) {

    state_t e_state_1 = *( mf_channel::pae_input_1 + i );

    if( e_state_1 == ATTACK ) {
      
      for( int16_t j = 0; j <= mf_channel::i_input_2_bound; j++ ) {

        state_t e_state_2 = *( mf_channel::pae_input_1 + *( mf_channel::pai_input_2 + j );
    
        if( e_state_2 == INACTIVE ) {

          *( mf_channel::pai_input_2 + j ) = i;

          break;
        }
      }
    }
  }
}
