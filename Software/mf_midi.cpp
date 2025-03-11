
#include "mf_types.h"
#include <mf_midi.h>
#include <driver/uart.h>

#define MIDI_UART                     UART_NUM_1
#define MIDI_TX_BUFFER_BOUND          63
#define MAX_VELOCITY                  127

char us_MIDI_TX_buffer[ MIDI_TX_BUFFER_BOUND + 1 ];



mf_midi::mf_midi() {}



void mf_midi::v_begin( midi_config_t* p_midi_config ) {

  mf_midi::p_midi_config = p_midi_config;

  mf_midi::us_midi_channel_base = mf_midi::p_midi_config->us_midi_channel_base;
  mf_midi::us_midi_channel_bound = mf_midi::p_midi_config->us_midi_channel_bound;
  mf_midi::us_midi_bank_msb = mf_midi::p_midi_config->us_midi_bank_msb;
  mf_midi::us_midi_bank_lsb = mf_midi::p_midi_config->us_midi_bank_lsb;
  mf_midi::us_midi_patch = mf_midi::p_midi_config->us_midi_patch;
  mf_midi::us_midi_out_pin = mf_midi::p_midi_config->us_midi_out_pin;
  mf_midi::us_midi_in_pin = mf_midi::p_midi_config->us_midi_in_pin;
  mf_midi::i_key_from_degree = mf_midi::p_midi_config->i_key_from_degree;
  mf_midi::i_bend_sensitivity = mf_midi::p_midi_config->i_bend_sensitivity;
  mf_midi::i_input_bound = mf_midi::p_midi_config->i_input_bound;
  mf_midi::pai_channel = mf_midi::p_midi_config->pai_channel;
  mf_midi::pae_input_1 = mf_midi::p_midi_config->pae_input_1;
  mf_midi::pai_input_2 = mf_midi::p_midi_config->pai_input_2;
  mf_midi::pai_input_3 = mf_midi::p_midi_config->pai_input_3;

// Configure the UART that drives the MIDI interface.

  esp_err_t return_code;
  
  uart_config_t MIDI_UART_config;

  MIDI_UART_config.baud_rate = 31250;
  MIDI_UART_config.data_bits = UART_DATA_8_BITS;
  MIDI_UART_config.parity    = UART_PARITY_DISABLE;
  MIDI_UART_config.stop_bits = UART_STOP_BITS_1;
  MIDI_UART_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  MIDI_UART_config.rx_flow_ctrl_thresh = 120;

  return_code = uart_param_config( MIDI_UART, &MIDI_UART_config);
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message and abort().

  return_code = uart_set_pin(
                MIDI_UART,
                mf_midi::us_midi_out_pin,
                mf_midi::us_midi_in_pin,
                UART_PIN_NO_CHANGE, // RTS not used.
                UART_PIN_NO_CHANGE ); // CTS not used.
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message and abort().

  uart_set_line_inverse( MIDI_UART, UART_SIGNAL_TXD_INV );

  return_code = uart_driver_install( MIDI_UART, 2048, 2048, 0, NULL, 0 );
  ESP_ERROR_CHECK( return_code ); // If not ESP_OK, then print an error message and abort().

// Configure channel_bound + 1 MIDI channels starting at channel_base.

  for( uint8_t i = 0; i <= mf_midi::us_midi_channel_bound; i++ ) {

    us_MIDI_TX_buffer[ 0 ] = 0xB0 | ( mf_midi::us_midi_channel_base + i );
    us_MIDI_TX_buffer[ 1 ] = 0x00;
    us_MIDI_TX_buffer[ 2 ] = mf_midi::us_midi_bank_msb;
    us_MIDI_TX_buffer[ 3 ] = 0xB0 | ( mf_midi::us_midi_channel_base + i );
    us_MIDI_TX_buffer[ 4 ] = 0x20;
    us_MIDI_TX_buffer[ 5 ] = mf_midi::us_midi_bank_lsb;

    uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 6 );

    us_MIDI_TX_buffer[ 0 ] = 0xC0 | ( mf_midi::us_midi_channel_base + i ); // Patch change.
    us_MIDI_TX_buffer[ 1 ] = mf_midi::us_midi_patch;

    uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 2 );

    us_MIDI_TX_buffer[ 0 ] = 0xB0 | ( mf_midi::us_midi_channel_base + i );
    us_MIDI_TX_buffer[ 1 ] = 0x78;
    us_MIDI_TX_buffer[ 2 ] = 0x00;

    uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 3 );

  }
}



void mf_midi::v_midi_from_message() {

  for( uint8_t i = 0; i <= mf_midi::us_midi_channel_bound; i++ ) {

    uint8_t us_channel = us_midi_channel_base + i;

    state_t e_state = *( mf_midi::pae_input_1 + *( mf_midi::pai_channel + i ));
    
    switch ( e_state ) {

    case ATTACK:
    
      mf_midi::note_off( us_channel, *( mf_midi::pai_input_2 + *( mf_midi::pai_channel + i ))->i_degree );
      mf_midi::note_on( us_channel, *( mf_midi::pai_input_2 + *( mf_midi::pai_channel + i ))->i_degree,
      *( mf_midi::pai_input_2 + *( mf_midi::pai_channel + i ))->i_velocity );
      break;
          
    case SUSTAIN:

      mf_midi::note_bend( us_channel, *( mf_midi::pai_input_3 + i ));
      break;
          
    case RELEASE:
    
      mf_midi::note_off( us_channel, *( mf_midi::pai_input_2 + *( mf_midi::pai_channel + i ))->i_degree );     
      break;
      
    }
  }
}



void mf_midi::note_on( int16_t i_channel, int16_t i_degree, int16_t i_velocity ) {

  i_velocity = ( i_velocity > MAX_VELOCITY ) ? MAX_VELOCITY : i_velocity;

  us_MIDI_TX_buffer[ 0 ] = 0x90 | ( mf_midi::us_midi_channel_base + i_channel ); // Note ...
  us_MIDI_TX_buffer[ 1 ] = (uint8_t)( i_degree + mf_midi::i_key_from_degree );
  us_MIDI_TX_buffer[ 2 ] = (uint8_t)i_velocity; // ... on.

  uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 3 );

}

void mf_midi::note_off( int16_t i_channel, int16_t i_degree ) {
  
  us_MIDI_TX_buffer[ 0 ] = 0x80 | ( mf_midi::us_midi_channel_base + i_channel );
  us_MIDI_TX_buffer[ 1 ] = (uint8_t)( i_degree + mf_midi::i_key_from_degree );
  us_MIDI_TX_buffer[ 2 ] = 0x40;

  uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 3 );
  
}


void mf_midi::note_bend( int16_t i_channel, int16_t i_bend ) {

// A MIDI message value of 0x00, 0x00 represents the maximum negative value of the pitch bend range.
// A MIDI message value of 0x00, 0x40 represents no pitch bend.
// A MIDI message value of 0x7F, 0x7F represents the maximum positive value of the pitch bend range.
// MIDI message value = i_bend * 2^14 / i_range.

  us_MIDI_TX_buffer[ 0 ] = 0xE0 | ( mf_midi::us_midi_channel_base + i_channel );
  us_MIDI_TX_buffer[ 1 ] = (uint8_t)(( mf_midi::i_bend_sensitivity * i_bend ) & 0x007F ); // LSB
  us_MIDI_TX_buffer[ 2 ] = (uint8_t)((( mf_midi::i_bend_sensitivity * i_bend ) & 0x3F80 ) >> 7 ); // MSB

  uart_write_bytes( MIDI_UART, us_MIDI_TX_buffer, 3 );
 
}
