#include <avr/io.h>
#include <util/delay.h>

int _bit_reset = 3;
int _bit_reddata = 2;
int _bit_copy = 4;
int _bit_coldata = 6;
int _bit_greendata = 5;
int _bit_enable = 7;

int _data_depth = 8;

struct char_data {

    int col0;
    int col1;
    int col2;
    int col3;
    int col4;
    int col5;
    int col6;
    int col7;
    int color_col0;
    int color_col1;
    int color_col2;
    int color_col3;
    int color_col4;
    int color_col5;
    int color_col6;
    int color_col7;

};

void display_column( int icol, int col_data, int color ) { 

  //int col_mapping[8] = {4, 5, 6, 7, 3, 2, 1, 0};
  int col_mapping[8] = {3, 2, 1, 0, 4, 5, 6, 7};
  //int col_mapping[8] = {0, 1, 2, 3,  7, 6, 5, 4};

  // disable output
  PORTB = PORTB | (0x1 << _bit_enable);

  // set the data
  for( int dd =0; dd <= _data_depth; dd++ ) {

      if( col_mapping[dd] == icol ) {
          PORTB = PORTB | (0x1 << _bit_coldata );
      }
      if( col_data & (0x1 << dd) & ~color ) {
          PORTB = PORTB | (0x1 << _bit_reddata ) ;
      }
      if( col_data & (0x1 << dd) & color ) {
          PORTB = PORTB | (0x1 << _bit_greendata ) ;
      }

      PORTB = PORTB | (0x1 << _bit_copy );

      // set data and copy signals to 0
      PORTB = PORTB & ~(0x1 << _bit_copy );
      PORTB = PORTB & ~(0x1 << _bit_coldata );
      PORTB = PORTB & ~(0x1 << _bit_reddata );
      PORTB = PORTB & ~(0x1 << _bit_greendata );
  }

  // enable output
  PORTB = PORTB & ~(0x1 << _bit_enable );
  
}

class pattern_container {

    int patterns[2048];

    public : 
    void add_pattern(int idx, int col1, int col2, int col3, int col4, int col5, int col6, int col7, int col8) {

	patterns[(8*idx)+0] = col1;
	patterns[(8*idx)+1] = col2;
	patterns[(8*idx)+2] = col3;
	patterns[(8*idx)+3] = col4;
	patterns[(8*idx)+4] = col5;
	patterns[(8*idx)+5] = col6;
	patterns[(8*idx)+6] = col7;
	patterns[(8*idx)+7] = col8;

    }

    char_data get ( int index, int color) {

       char_data cd;
       cd.col0 = patterns[(8*index)+0];
       cd.col1 = patterns[(8*index)+1];
       cd.col2 = patterns[(8*index)+2];
       cd.col3 = patterns[(8*index)+3];
       cd.col4 = patterns[(8*index)+4];
       cd.col5 = patterns[(8*index)+5];
       cd.col6 = patterns[(8*index)+6];
       cd.col7 = patterns[(8*index)+7];
       cd.color_col0 = 0x00;
       cd.color_col1 = 0x00;
       cd.color_col2 = 0x00;
       cd.color_col3 = 0x00;
       cd.color_col4 = 0x00;
       cd.color_col5 = 0x00;
       cd.color_col6 = 0x00;
       cd.color_col7 = 0x00;
       if( 0x01 & color ) cd.color_col0 = 0xff;
       if( 0x02 & color) cd.color_col1 = 0xff;
       if( 0x04 & color) cd.color_col2 = 0xff;
       if( 0x08 & color) cd.color_col3 = 0xff;
       if( 0x10 & color) cd.color_col4 = 0xff;
       if( 0x20 & color) cd.color_col5 = 0xff;
       if( 0x40 & color) cd.color_col6 = 0xff;
       if( 0x80 & color) cd.color_col7 = 0xff;

       return cd;
    	
    }
};


void delay_ms(int ms)
{
    while (ms--) {
        _delay_us(1000);  // one millisecond
    }
}

//void display_pattern( int col1, int col2, int col3, int col4, int col5, int col6, int col7, int col8, int time ) {
//	
//  for(int i = 0; i < time ; ++i ) {
//      display_column(0, col1 );
//      _delay_us(100);
//      display_column(1, col2 );
//      _delay_us(100);
//      display_column(2, col3 );
//      _delay_us(100);
//      display_column(3, col4 );
//      _delay_us(100);
//      display_column(4, col5 );
//      _delay_us(100);
//      display_column(5, col6 );
//      _delay_us(100);
//      display_column(6, col7 );
//      _delay_us(100);
//      display_column(7, col8 );
//      _delay_us(100);
//  }
//
//}

void display_pattern( int col0, int col_col0, int col1, int col_col1,
                      int col2, int col_col2, int col3, int col_col3,
                      int col4, int col_col4, int col5, int col_col5,
                      int col6, int col_col6, int col7, int col_col7, int time ) {

    for(int i = 0; i < time ; ++i ) {
        display_column(0, col0, col_col0 );
        _delay_us(100);
        display_column(1, col1, col_col1 );
        _delay_us(100);
        display_column(2, col2, col_col2 );
        _delay_us(100);
        display_column(3, col3, col_col3 );
        _delay_us(100);
        display_column(4, col4, col_col4 );
        _delay_us(100);
        display_column(5, col5, col_col5 );
        _delay_us(100);
        display_column(6, col6, col_col6 );
        _delay_us(100);
        display_column(7, col7, col_col7 );
        _delay_us(100);
    }

}

void display_pattern( char_data data, int time ) {
    
    for(int i = 0; i < time ; ++i ) {
        display_column(0, data.col0, data.color_col0 );
        _delay_us(100);
        display_column(1, data.col1, data.color_col1 );
        _delay_us(100);
        display_column(2, data.col2, data.color_col2 );
        _delay_us(100);
        display_column(3, data.col3, data.color_col3 );
        _delay_us(100);
        display_column(4, data.col4, data.color_col4 );
        _delay_us(100);
        display_column(5, data.col5, data.color_col5 );
        _delay_us(100);
        display_column(6, data.col6, data.color_col6 );
        _delay_us(100);
        display_column(7, data.col7, data.color_col7 );
        _delay_us(100);
    }

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

  pattern_container pc;
  //pc.add_pattern(0, 0x0, 0xff, 0x81, 0x81, 0x81, 0x81, 0xff, 0x00 );
  pc.add_pattern(0, 0x0, 0x00, 0x7e, 0x42, 0x42, 0x7e, 0x00, 0x00 );
  pc.add_pattern(1, 0x0, 0x00, 0x21, 0x41, 0xff, 0x01, 0x01, 0x00 );
  pc.add_pattern(2, 0x0, 0x00, 0x8f, 0x91, 0x91, 0xf1, 0x00, 0x00 );
  pc.add_pattern(3, 0x0, 0x00, 0x41, 0x49, 0x49, 0x7f, 0x00, 0x00 );
  pc.add_pattern(4, 0x0, 0x00, 0x78, 0x08, 0x08, 0x7f, 0x00, 0x00 );
  pc.add_pattern(5, 0x0, 0x00, 0xf1, 0x91, 0x91, 0x1f, 0x00, 0x00 );
  pc.add_pattern(6, 0x0, 0x0e, 0x11, 0x29, 0x49, 0x87, 0x00, 0x00 );
  pc.add_pattern(7, 0x0, 0x00, 0x40, 0x40, 0x40, 0x7f, 0x00, 0x00 );
  pc.add_pattern(8, 0x0, 0x00, 0x7f, 0x49, 0x49, 0x7f, 0x00, 0x00 );
  pc.add_pattern(9, 0x0, 0x00, 0x70, 0x48, 0x48, 0x7f, 0x00, 0x00 );

  //int pat_4[8] = {4, 5, 6, 7, 3, 2, 1, 0};

  //delay_ms(1000);
  //display_pattern( 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb );
  while (1) {

    //display_pattern( 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb );

    display_pattern( 0x3c, 0xff, 0x7e, 0xf7, 0xff, 0x9b, 0xff, 0xfb, 0xff, 0xfb, 0xff, 0x9b, 0x7e, 0xf7, 0x3c, 0xff, 10000 );
    
    //display_pattern( 0x0, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x42, 0x00, 0x42, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 100 );
    //display_pattern( 0x0, 0xff, 0x00, 0xff, 0x21, 0xff, 0x41, 0xff, 0xff, 0xff, 0x01, 0xff, 0x01, 0xff, 0x00, 0xff, 100 );
    //display_pattern( 0x0, 0x00, 0x00, 0x00, 0x8f, 0x00, 0x91, 0x00, 0x91, 0x00, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00, 100 );
    //display_pattern( 0x0, 0xff, 0x00, 0xff, 0x41, 0xff, 0x49, 0xff, 0x49, 0xff, 0x7f, 0xff, 0x00, 0xff, 0x00, 0xff, 100 );
    //display_pattern( 0x0, 0x00, 0x00, 0x00, 0x78, 0x00, 0x08, 0x00, 0x08, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 100 );
    //display_pattern( 0x0, 0xff, 0x00, 0xff, 0xf1, 0xff, 0x91, 0xff, 0x91, 0xff, 0x1f, 0xff, 0x00, 0xff, 0x00, 0xff, 100 );
    //display_pattern( 0x0, 0x00, 0x0e, 0x00, 0x11, 0x00, 0x29, 0x00, 0x49, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 100 );
    //display_pattern( 0x0, 0xff, 0x00, 0xff, 0x40, 0xff, 0x40, 0xff, 0x40, 0xff, 0x7f, 0xff, 0x00, 0xff, 0x00, 0xff, 100 );
    //display_pattern( 0x0, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x49, 0x00, 0x49, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 100 );
    //display_pattern( 0x0, 0xff, 0x00, 0xff, 0x70, 0xff, 0x48, 0xff, 0x48, 0xff, 0x7f, 0xff, 0x00, 0xff, 0x00, 0xff, 100 );

    //display_pattern( pc.get(0, 0x00), 100 );
    //display_pattern( pc.get(1, 0xff), 100 );
    //display_pattern( pc.get(2, 0x00), 100 );
    //display_pattern( pc.get(3, 0xff), 100 );
    //display_pattern( pc.get(4, 0x00), 100 );
    //display_pattern( pc.get(5, 0xff), 100 );
    //display_pattern( pc.get(6, 0x00), 100 );
    //display_pattern( pc.get(7, 0xff), 100 );
    //display_pattern( pc.get(8, 0x00), 100 );
    //display_pattern( pc.get(9, 0xff), 100 );
    //delay_ms(2000);
    //display_pattern2( 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 );
    //delay_ms(2000);

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

