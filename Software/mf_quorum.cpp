
#include "mf_types.h"
#include "mf_quorum.h"

mf_quorum::mf_quorum() {}

void mf_quorum::v_begin( quorum_config_t* p_quorum_config ) {
  
  mf_quorum::p_quorum_config = p_quorum_config;

  mf_quorum::d_quorum_level = mf_quorum::p_quorum_config->d_quorum;
  mf_quorum::i_quorum_threshold = mf_quorum::p_quorum_config->i_threshold;
  mf_quorum::pi_dft_peak = mf_quorum::p_quorum_config->pi_dft_peak;
  mf_quorum::i_input_bound = mf_quorum::p_quorum_config->i_input_bound;
  mf_quorum::pai_input = mf_quorum::p_quorum_config->pai_input;
  mf_quorum::i_output_bound = mf_quorum::p_quorum_config->i_output_bound;
  mf_quorum::pai_output = mf_quorum::p_quorum_config->pai_output;

}

void mf_quorum::v_quorum_from_dft() {

int16_t i_total_value;
int16_t i_partial_sum;
int16_t i_quorum_bound; // The max index of the entry required to exceed quorum.

int16_t ai_input_degree[ MODEL_BOUND + 1 ];
int16_t ai_input_value[ MODEL_BOUND + 1 ];
int16_t i_temp_degree;
int16_t i_temp_value;

  i_total_value = 0;
  i_partial_sum = 0;
  i_quorum_bound = 0;

  for( int16_t i = 0; i <= mf_quorum::i_output_bound; i++ ) {

    *( mf_quorum::pai_output + i ) = 0;

  } // All output degrees are intially marked 0.
 
  if( *mf_quorum::pi_dft_peak > mf_quorum::i_quorum_threshold ) { // Peak spectrum energy meets the threshold.
  
    for( int16_t i = 0; i <= mf_quorum::i_input_bound; i++ ) {

      ai_input_degree[ i ] = i;
      ai_input_value[ i ] = *( mf_quorum::pai_input + i );
      i_total_value += ai_input_value[ i ];

    }
    
// ai_input_degree contains the integers from 0 to mf_quorum::i_input_bound in ascending order.
// ai_input_value is a copy of ai_input, and
// i_total_value is the sum of the values of all input entries.

    for( int16_t i = 0; i <= mf_quorum::i_input_bound; i++ ) {

      for( int16_t j = i + 1; j<= mf_quorum::i_input_bound; j++ ) {

        if( ai_input_value[ j ] > ai_input_value[ i ] ) {

          i_temp_degree = ai_input_degree[ i ];
          i_temp_value = ai_input_value[ i ];

          ai_input_degree[ i ] = ai_input_degree[ j ];
          ai_input_value[ i ] = ai_input_value[ j ];

          ai_input_degree[ j ] = i_temp_degree;
          ai_input_value[ j ] = i_temp_value;

        }
      }
    }

// ai_input_value contains the values of ai_input ordered by decreasing value.
// ai_input_degree contains the corresponding indices.

    for( int16_t i = 0; i <= mf_quorum::i_output_bound; i++ ) {

      i_partial_sum += ai_input_value[ i ];
 
      if( i_partial_sum > (int16_t)( mf_quorum::d_quorum_level * (double)i_total_value )) {

        i_quorum_bound = i;

        break;

      }
    } // i_quorum_bound is now the smallest number of entries required to achieve quorum minus 1.
     
    for( int16_t i = 0; i <= i_quorum_bound; i++ ) {

      *( mf_quorum::pai_output + ai_input_degree[ i ] ) = 1;

    } // Now the smallest set of output entries that form quorum are marked 1, while all others are marked 0.
  }
}
