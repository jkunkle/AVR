#include <avr/io.h>
#include <util/delay.h>

int _bit_reset = 3;
int _bit_coldata = 2;
int _bit_colcopy = 4;
int _bit_rowdata = 6;
int _bit_rowcopy = 5;
int _bit_enable = 7;
int _data_depth = 8;


void delay_ms(int ms)
{
    while (ms--) {
        _delay_us(1000);  // one millisecond
    }
}

void display_column( int icol, int col_data ) { 

  int col_mapping[8] = {4, 5, 6, 7, 3, 2, 1, 0};

  // disable output
  PORTB = PORTB | (0x1 << _bit_enable);

  // set the column data
  for( int dd =0; dd <= _data_depth; dd++ ) {

      if( col_data & (0x1 << dd) ) {
        PORTB = PORTB | (0x1 << _bit_coldata ) ;
      }
      PORTB = PORTB | (0x1 << _bit_colcopy );
      //delay_ms(1000);

      // set data and copy signals to 0
      PORTB = PORTB & ~(0x1 << _bit_colcopy );
      PORTB = PORTB & ~(0x1 << _bit_coldata );
      //delay_ms(1000);
  }
  // set the row
  for( int dd =0; dd <= _data_depth; dd++ ) {

    //delay_ms(1000);
    if( col_mapping[dd] == icol ) {
      PORTB = PORTB | (0x1 << _bit_rowdata );
      //delay_ms(1000);
    }
    PORTB = PORTB | (0x1 << _bit_rowcopy );
    //delay_ms(1000);

    // set data and copy signals to 0
    PORTB = PORTB & ~(0x1 << _bit_rowcopy );
    //delay_ms(1000);
    PORTB = PORTB & ~(0x1 << _bit_rowdata );
    //delay_ms(1000);
  }
  PORTB = PORTB & ~(0x1 << _bit_enable );
  
}

void display_pattern( int col1, int col2, int col3, int col4, int col5, int col6, int col7, int col8 ) {
	
  display_column(0, col1 );
  _delay_us(100);
  display_column(1, col2 );
  _delay_us(100);
  display_column(2, col3 );
  _delay_us(100);
  display_column(3, col4 );
  _delay_us(100);
  display_column(4, col5 );
  _delay_us(100);
  display_column(5, col6 );
  _delay_us(100);
  display_column(6, col7 );
  _delay_us(100);
  display_column(7, col8 );
  _delay_us(100);

}
    
int main(void)
{
  DDRB = 0xfc;

  // start with reset off
  PORTB = 0x01 << _bit_reset;
  //delay_ms(1000);

  // Reset shift registers
  PORTB = 0x00; 
  // keep the reset active for some time (can be tuned)
  //delay_ms(1000);
  _delay_us(1);
  PORTB = 0x01 << _bit_reset;

  //delay_ms(1000);
  //display_pattern( 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb );
  while (1) {

    //display_pattern( 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb );
    //display_pattern( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 );
    display_pattern( 0x0, 0x0, 0x78, 0x08, 0x08, 0x7f, 0x00, 0x00 );
    delay_ms(2000);
    display_pattern( 0x0, 0x0, 0x78, 0x08, 0x08, 0x7f, 0x00, 0x00 );

    //display_column( 0, 0xbb);
    //delay_ms(10000);
    //display_column( 1, 0x11);
    //delay_ms(10000);
    //display_column( 2,0xbb);
    //delay_ms(10000);
    //display_column( 3, 0xcc);
    //delay_ms(10000);
    //display_column( 4, 0xd1);
    //delay_ms(10000);
    //display_column( 5, 0x33);
    //delay_ms(10000);
    //display_column( 6, 0x74 );
    //delay_ms(10000);
    //display_column( 7, 0xbb );
    //delay_ms(10000);

  }
}

