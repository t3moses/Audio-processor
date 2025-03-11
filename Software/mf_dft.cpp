
#include "mf_types.h"
#include "mf_dft.h"
#include "mf_audio_sampler.h"

adc_config_t _adc_config;
adc_config_t* _p_adc_config = &_adc_config;
adc_pins_t _adc_pins;
adc_pins_t* _p_adc_pins = &_adc_pins;

mf_audio_sampler sampler;

mf_dft::mf_dft() {}

void mf_dft::v_begin( dft_config_t* p_dft_config ) {

  mf_dft::p_dft_config = p_dft_config;

  mf_dft::d_dft_root_frequency = mf_dft::p_dft_config->d_dft_root_frequency;
  mf_dft::i_ref_cycle_bound = mf_dft::p_dft_config->i_ref_cycle_bound;
  mf_dft::i_ref_datapoint_bound = mf_dft::p_dft_config->i_ref_datapoint_bound;
  mf_dft::pi_audio_peak = mf_dft::p_dft_config->pi_audio_peak;
  mf_dft::pi_dft_peak = mf_dft::p_dft_config->pi_dft_peak;
  mf_dft::i_dft_bound = mf_dft::p_dft_config->i_dft_bound;
  mf_dft::i_dewindow_bound = mf_dft::p_dft_config->i_dewindow_bound;
  mf_dft::i_atten = mf_dft::p_dft_config->i_atten;
  mf_dft::pae_state = mf_dft::p_dft_config->pae_state;
  mf_dft::i_bend_sensitivity = mf_dft::p_dft_config->i_bend_sensitivity;
  mf_dft::d_pole = mf_dft::p_dft_config->d_pole;
  mf_dft::pai_raw_dft = mf_dft::p_dft_config->pai_raw_dft;
  mf_dft::pad_pitch_bend = mf_dft::p_dft_config->pad_pitch_bend;

  mf_dft::i_audio_index = 0;

  adc_config_t adc_config = {
  
    .i_audio_index_max = AUDIO_BOUND,
    .pi_audio_index = mf_dft::pi_audio_index,
    .pi_audio = mf_dft::pai_audio

  };

  adc_pins_t adc_pins = {
  
    .i_pin_num_cs = mf_dft::p_dft_config->i_pin_num_cs,
    .i_pin_num_miso = mf_dft::p_dft_config->i_pin_num_miso,
    .i_pin_num_clk = mf_dft::p_dft_config->i_pin_num_clk

  };

  mf_dft::i_raw_dft_bound = mf_dft::i_dft_bound + 2 * mf_dft::i_dewindow_bound;
  mf_dft::d_raw_dft_root_frequency = mf_dft::d_dft_root_frequency * pow( 2.0, -(double)mf_dft::i_dewindow_bound / 12.0 ); 

  mf_dft::d_loop_forward_path_constant = -(double)mf_dft::i_bend_message_sensitivity * (1.0 - mf_dft::d_pole ) / ( 0.02 * PI * ( pow( 2.0, 1.0 / 12.0 ) - 1.0 ) * (double)mf_dft::i_ref_cycle_bound );
  mf_dft::d_loop_reverse_path_constant = mf_dft::d_pole / ( 0.02 * PI * ( pow( 2.0, 1.0 / 12.0 ) - 1.0 ) * (double)mf_dft::i_ref_cycle_bound * mf_dft::d_loop_forward_path_constant );
 
  sampler.v_begin( &adc_pins, &adc_config );
  
  mf_dft::v_set_up_reference_values();
    
  delay( 200 ); // Wait for the input buffer to fill.
  
  mf_dft::v_measure_adc_offset();
  
}



void mf_dft::v_set_up_reference_values() {

// Build arrays of datapoints for one cycle of inphase and quadrature reference signals.

  double d_radians_per_reference_interval = 2.0 * PI / (double)( mf_dft::i_ref_datapoint_bound + 1 );

  for ( uint8_t i = 0; i <= mf_dft::i_ref_datapoint_bound; i++ ) {

    mf_dft::i_ref_inphase_value[ i ] = (int16_t)( LSB_PER_VOLT * REFERENCE_AMPLITUDE * cos( (double)i * d_radians_per_reference_interval ));
    mf_dft::i_ref_quadrature_value[ i ] = (int16_t)( LSB_PER_VOLT * REFERENCE_AMPLITUDE * sin( (double)i * d_radians_per_reference_interval ));

  }
}



void mf_dft::v_measure_adc_offset() {

// Take ADC_OFFSET_SAMPLE_BOUND samples of audio input and calculate the mean.

  int32_t l_interim_sum = 0;

  for (int16_t i = 0; i <= ADC_OFFSET_SAMPLE_BOUND ; i++ ) {
    l_interim_sum = l_interim_sum + (int32_t)mf_dft::ai_audio[ i ];
  }

  mf_dft::i_adc_offset = (int16_t)( l_interim_sum / ( ADC_OFFSET_SAMPLE_BOUND + 1 ));
  
}



void mf_dft::v_sample_from_audio( ) {

// Use linear interpolation to obtain an audio value corresponding to the phase of the reference signal.
// mf_dft::i_write_index_copy points to the last (written) audio sample.

  double d_ideal_sample_time_copy = mf_dft::d_ideal_sample_time;
  double d_fractional_part = modf( d_ideal_sample_time_copy, &d_ideal_sample_time_copy );

  mf_dft::i_best_sample = (int16_t)d_ideal_sample_time_copy;

  mf_dft::i_read_index_after = mf_dft::i_write_index_copy - mf_dft::i_best_sample;
  mf_dft::i_read_index_after = ( mf_dft::i_read_index_after >= 0 ) ? mf_dft::i_read_index_after : mf_dft::i_read_index_after + 1 + AUDIO_BOUND;

  mf_dft::i_read_index_before = mf_dft::i_read_index_after - 1;
  mf_dft::i_read_index_before = ( mf_dft::i_read_index_before >= 0 ) ? mf_dft::i_read_index_before : mf_dft::i_read_index_before + 1 + AUDIO_BOUND;

  noInterrupts();
  mf_dft::i_raw_sample_before = mf_dft::ai_audio[ mf_dft::i_read_index_before ];
  mf_dft::i_raw_sample_after = mf_dft::ai_audio[ mf_dft::i_read_index_after ];
  interrupts();

  mf_dft::i_audio_sample_value = mf_dft::i_raw_sample_after + (int16_t)((double)( mf_dft::i_raw_sample_before - mf_dft::i_raw_sample_after ) * d_fractional_part ) - mf_dft::i_adc_offset;

}



void mf_dft::v_note_pitch_offset_from_cycle_phase_array() {

// Calculate the audio pitch offset relative to the reference frequency for one note,
// by adding the phase angle increases over each reference cycle of a complete window.

int16_t i_quadrant;
double d_phase_difference; // The audio phase angle increase relative to the reference over one reference cycle.

  mf_dft::d_pitch_offset = 0.0;

  for( mf_dft::i_ref_cycle = 0; mf_dft::i_ref_cycle <= mf_dft::i_ref_cycle_bound; mf_dft::i_ref_cycle++ ) {
      
    if( *( mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle ) != 0 ) { // Avoid divide by zero.

      *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) = atan( (double)( *(mf_dft::pal_cycle_quadrature + mf_dft::i_ref_cycle )) / 
        (double)( *(mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle )));

// atan() returns a value between -PI/2 and +PI/2.  Map it to the range 0 .. 2 PI.

      if( *(mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle ) > 0 && *(mf_dft::pal_cycle_quadrature ) >= 0 ) i_quadrant = 1;
        else if ( *(mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle ) < 0 && *(mf_dft::pal_cycle_quadrature ) >= 0 ) i_quadrant = 2;
          else if ( *(mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle ) < 0 && *(mf_dft::pal_cycle_quadrature ) < 0 ) i_quadrant = 3;
            else i_quadrant = 4;

      switch( i_quadrant ) {
      case 1: *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) += 0.0; break; // 0 <= atan() < +PI/2, so 0 <= d_loop_reverse_path_constantycle_phase < +PI/2.
      case 2: *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) += PI; break; // -PI/2 < atan() <= 0, so +PI/2 < d_loop_reverse_path_constantycle_phase <= +PI.
      case 3: *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) += PI; break; // 0 <= atan() < +PI/2, so +PI <= d_loop_reverse_path_constantycle_phase < +3PI/2.
      case 4: *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) += 2.0 * PI; break; // -PI/2 < atan() < 0, so +3PI/2 < d_loop_reverse_path_constantycle_phase < +2PI.
 
      }
    }
    else if( *( mf_dft::pal_cycle_quadrature + mf_dft::i_ref_cycle ) >= 0 ) *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) = PI / 2.0;
      else *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) = -PI / 2.0;

    if( mf_dft::i_ref_cycle > 0 ) {
    
      d_phase_difference = *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle ) - *( mf_dft::pad_cycle_phase + mf_dft::i_ref_cycle - 1 );
      if( d_phase_difference > PI ) d_phase_difference -= ( 2.0 * PI ); else if( d_phase_difference < -PI ) d_phase_difference += ( 2.0 * PI );
      mf_dft::d_pitch_offset += d_phase_difference;

    }
  }
}




void mf_dft::v_raw_dft_from_audio() {

  *mf_dft::pi_audio_peak = 0;
  *mf_dft::pi_dft_peak = 0;
 
  for ( mf_dft::i_degree = 0; mf_dft::i_degree <= mf_dft::i_raw_dft_bound; mf_dft::i_degree++ ) {

// Analyse the power in one note, starting with the lowest.

    mf_dft::d_degree_factor = pow( 2.0, ( (double)( 100 * mf_dft::i_degree ) + *( mf_dft::pad_pitch_correction + mf_dft::i_degree )) / 1200.0 );
    mf_dft::d_ref_frequency = mf_dft::d_dft_root_frequency * mf_dft::d_degree_factor;
    mf_dft::d_subsample_interval = (double)SAMPLES_PER_SECOND /
      ( (double)( mf_dft::i_ref_datapoint_bound + 1 ) * mf_dft::d_ref_frequency );
    mf_dft::d_ideal_sample_time = 0.0;
    
    mf_dft::l_degree_inphase_component = 0;
    mf_dft::l_degree_quadrature_component = 0;

    noInterrupts(); mf_dft::i_write_index_copy = mf_dft::i_audio_index; interrupts();

    for ( mf_dft::i_ref_cycle = 0; mf_dft::i_ref_cycle <= mf_dft::i_ref_cycle_bound; mf_dft::i_ref_cycle++ ) {

      mf_dft::l_cycle_inphase_component = 0;
      mf_dft::l_cycle_quadrature_component = 0;

      for ( mf_dft::i_ref_datapoint = 0; mf_dft::i_ref_datapoint <= mf_dft::i_ref_datapoint_bound; mf_dft::i_ref_datapoint++ ) {

        mf_dft::v_sample_from_audio( ); // Choose mf_dft::i_audio_sample_value, the best audio sample to match 
                                        // i_ref_cycle and i_ref_datapoint for the current note.
                                        // This also updates l_audio_max.

        mf_dft::d_ideal_sample_time += mf_dft::d_subsample_interval;

// Accumulate inphase and quadrature powers for each datapoint in the subframe.
// l_degree_quadrature_component and l_quadrature_component are in the range +/- log2 ( i_ref_cycle_bound ) + 2 * ( ADC_BITS - 1 ) bits.
// i.e. 25 bits for i_ref_cycle_bound = 10 and ADC_BITS = 12.

        mf_dft::l_cycle_inphase_component += (int32_t)mf_dft::i_audio_sample_value * 
        (int32_t)mf_dft::i_ref_inphase_value[ i_ref_datapoint ];
        
        mf_dft::l_cycle_quadrature_component += (int32_t)mf_dft::i_audio_sample_value * 
        (int32_t)mf_dft::i_ref_quadrature_value[ i_ref_datapoint ];

// Set i_audio_peak to the strength of the strongest entry in the audio array.

        if( mf_dft::i_audio_sample_value > *mf_dft::pi_audio_peak ) {
          *mf_dft::pi_audio_peak = mf_dft::i_audio_sample_value;
          
        }
      } // Finished calculating the inphase and quadrature components in one reference cycle.

      mf_dft::l_degree_inphase_component += mf_dft::l_cycle_inphase_component;
      mf_dft::l_degree_quadrature_component += mf_dft::l_cycle_quadrature_component;

      *( mf_dft::pal_cycle_inphase + mf_dft::i_ref_cycle ) = mf_dft::l_cycle_inphase_component;
      *( mf_dft::pal_cycle_quadrature + mf_dft::i_ref_cycle ) = mf_dft::l_cycle_quadrature_component;
    
    } // Finished calculating the power in one note.

// Reduce l_degree_inphase_component and l_quadrature_component to 14-bit signed integers.

    mf_dft::l_degree_inphase_component /= (int32_t)( mf_dft::i_atten * mf_dft::i_ref_datapoint_bound * mf_dft::i_ref_cycle_bound );
    mf_dft::l_degree_quadrature_component /= (int32_t)( mf_dft::i_atten * mf_dft::i_ref_datapoint_bound * mf_dft::i_ref_cycle_bound );

// Only if overflow has not occured ...

    if( mf_dft::l_degree_inphase_component < 16384 &&
     mf_dft::l_degree_inphase_component > -16384 &&
     mf_dft::l_degree_quadrature_component < 16384 &&
     mf_dft::l_degree_quadrature_component > -16384 ) {

// Calculate the note power as the sum of the squares of the inphase and quadrature components, and then apply overall scaling.

      *( mf_dft::pai_raw_dft + mf_dft::i_degree ) = (int16_t)( 
        mf_dft::d_degree_factor * 
        (double)((
        mf_dft::l_degree_inphase_component * 
        mf_dft::l_degree_inphase_component + 
        mf_dft::l_degree_quadrature_component * 
        mf_dft::l_degree_quadrature_component ) /
        65536 ));

    }

    i_dft_index = i_degree - i_dewindow_bound;

    if( ( i_dft_index >= 0 ) &&
        ( i_dft_index <= 0 i_dft_bound ) &&
        ( *( mf_dft::pae_state + mf_dft::i_dft_index ) == SUSTAIN )) {
          
      mf_dft::v_note_pitch_offset_from_cycle_phase_array();

      *( mf_dft::pad_pitch_bend + mf_dft::i_dft_index ) = mf_dft::d_loop_forward_path_constant * mf_dft::d_pitch_offset;
      *( mf_dft::pad_pitch_correction + mf_dft::i_dft_index ) = mf_dft::d_loop_reverse_path_constant * ( *( mf_dft::pad_pitch_bend + mf_dft::i_dft_index ));

    }
 
  } // Finished calculating the power in all notes.
    
// Set i_dft_peak to the strength of the biggest entry in the raw DFT.

  if( *( mf_dft::pai_raw_dft + mf_dft::i_degree ) > *mf_dft::pi_dft_peak ) *mf_dft::pi_dft_peak = *( mf_dft::pai_raw_dft + mf_dft::i_degree );

}
  
