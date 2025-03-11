
#include "mf_types.h"
#include "mf_sieve.h"
#include "mf_sieve_matrix.h"

mf_sieve::mf_sieve() { }

void mf_sieve::v_begin( sieve_config_t* p_sieve_config ) {

  mf_sieve::p_sieve_config = p_sieve_config;
    
  mf_sieve::i_input_bound = mf_sieve::p_sieve_config->i_input_bound;
  mf_sieve::pai_input = mf_sieve::p_sieve_config->pai_input;
  mf_sieve::i_output_bound = mf_sieve::p_sieve_config->i_output_bound;
  mf_sieve::pai_output = mf_sieve::p_sieve_config->pai_output;

}

void mf_sieve::v_sieve_from_quorum() {

// ai_output is a column vector formed by premultiplying ai_input by ai_sieve_matrix.

  for( int16_t i_row = 0; i_row <= mf_sieve::i_input_bound; i_row++ ) {
    
    *( mf_sieve::pai_output + i_row ) = 0;
    
    for( int16_t i_column = 0; i_column <= mf_sieve::i_input_bound; i_column++ ) {

      *( mf_sieve::pai_output + i_row ) += pai_sieve_matrix[ ( mf_sieve::i_input_bound + 1 ) * i_row + i_column ] * *( mf_sieve::pai_input + i_column );

    }
  }
}

    
