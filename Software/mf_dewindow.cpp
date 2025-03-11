
#include "mf_types.h"
#include "mf_dewindow.h"
#include "mf_dw_matrix_3_9.h"

mf_dewindow::mf_dewindow() {}

void mf_dewindow::v_begin( dewindow_config_t* p_dewindow_config ) {
  
  mf_dewindow::p_dewindow_config = p_dewindow_config;
  
  mf_dewindow::i_raw_dft_bound = mf_dewindow::p_dewindow_config->i_input_bound;
  mf_dewindow::i_dewindow_bound = mf_dewindow::p_dewindow_config->i_dewindow_bound;
  mf_dewindow::pai_raw_dft = mf_dewindow::p_dewindow_config->pai_raw_dft;
  mf_dewindow::pai_dft = mf_dewindow::p_dewindow_config->pai_dft;

  mf_dewindow::i_dft_bound = mf_dewindow::i_raw_dft_bound = 2 * mf_dewindow::i_dewindow_bound + 1;
  
}



void mf_dewindow::v_dewindow_baseband() {

  for( int16_t i = 0; i <= mf_dewindow::i_dft_bound; i++ ) {

    *( mf_dewindow::pai_dft + i ) = 0; // Zero the accumulator for each note.

  }
   
  for( int16_t i = 0; i <= mf_dewindow::i_raw_dft_bound - mf_dewindow::i_dewindow_bound; i++ ) {
       
    for( int16_t i_row = 0; i_row <= mf_dewindow::i_dewindow_bound; i_row++ ) {
      
      if((( i + i_row ) >= mf_dewindow::i_dewindow_bound ) &&
        (( i + i_row ) <= ( mf_dewindow::i_input_bound - mf_dewindow::i_dewindow_bound ))) {

        for( int16_t i_column = 0; i_column <= mf_dewindow::i_dewindow_bound; i_column++ ) {

          *( mf_dewindow::pai_dft + i + i_row - mf_dewindow::i_dewindow_bound ) +=
          (int16_t)( pad_dewindow[ ( mf_dewindow::i_dewindow_bound + 1 ) * i_row + i_column ] * 
          (double)( *( mf_dewindow::pai_raw_dft + i + i_column )));

        }
      }
    }
  }
  for( int16_t i = 0; i <= mf_dewindow::i_dft_bound; i++ ) {

    *( mf_dewindow::pai_dft + i ) = ( *( mf_dewindow::pai_dft + i ) >= 0 ) ? *( mf_dewindow::pai_dft + i ) : 0;
  }
}
