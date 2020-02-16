#include <avr/io.h>
#include <util/delay.h>

int _bit_reset = 3;
int _bit_reddata = 2;
int _bit_reddata_2 = 0;
int _bit_copy = 4;
int _bit_coldata = 6;
int _bit_coldata_2 = 1;
int _bit_enable = 7;

int _data_depth = 8;

uint8_t patterns[10][8] = { 
             { 0x00, 0x0f, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0f },
             { 0x00, 0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x06 },
             { 0x00, 0x0f, 0x08, 0x08, 0x0f, 0x01, 0x01, 0x0f },
             { 0x00, 0x0f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x0f },
             { 0x00, 0x01, 0x01, 0x01, 0x0f, 0x09, 0x09, 0x09 },
             { 0x00, 0x0f, 0x01, 0x01, 0x0f, 0x08, 0x08, 0x0f },
             { 0x00, 0x0f, 0x09, 0x09, 0x0f, 0x08, 0x08, 0x0f },
             { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0f },
             { 0x00, 0x0f, 0x09, 0x09, 0x0f, 0x09, 0x09, 0x0f },
             { 0x00, 0x01, 0x01, 0x01, 0x0f, 0x09, 0x09, 0x0f }
};

void display_column( int icol, int col_data, int icol2, int col_data2 ) { 

  //int col_mapping[8] = {3, 2, 1, 0, 4, 5, 6, 7};
  //int col_mapping[8] = {3, 2, 1, 0, 4, 5, 6, 7};
  int col_mapping[8] = {0, 1, 2, 3, 7, 6, 5, 4};

  // disable output
  PORTB = PORTB | (0x1 << _bit_enable);

  // set the data
  for( int dd =0; dd <= _data_depth; dd++ ) {
  //for( int dd =_data_depth-1; dd > 0; dd-- ) {

      if( col_mapping[dd] == icol ) {
          PORTB = PORTB | (0x1 << _bit_coldata );
      }
      if( col_mapping[dd] == icol2 ) {
          PORTB = PORTB | (0x1 << _bit_coldata_2 );
      }
      if( col_data & (0x1 << dd) ) {
          PORTB = PORTB | (0x1 << _bit_reddata ) ;
      }
      if( col_data2 & (0x1 << dd) ) {
          PORTB = PORTB | (0x1 << _bit_reddata_2 ) ;
      }

      PORTB = PORTB | (0x1 << _bit_copy );

      // set data and copy signals to 0
      PORTB = PORTB & ~(0x1 << _bit_copy );
      PORTB = PORTB & ~(0x1 << _bit_coldata );
      PORTB = PORTB & ~(0x1 << _bit_reddata );
      PORTB = PORTB & ~(0x1 << _bit_coldata_2 );
      PORTB = PORTB & ~(0x1 << _bit_reddata_2 );
  }

  // enable output
  PORTB = PORTB & ~(0x1 << _bit_enable );
  
}

void delay_ms(int ms)
{
    while (ms--) {
        _delay_us(1000);  // one millisecond
    }
}

void display_pattern( int col0, int col1, 
                      int col2, int col3, 
                      int col4, int col5, 
                      int col6, int col7, 
                      int col0_2, int col1_2,
                      int col2_2, int col3_2,
                      int col4_2, int col5_2,
                      int col6_2, int col7_2,
                      int time ) {

    for(int i = 0; i < time ; ++i ) {
        display_column(0, col0,0, col0_2);
        _delay_us(10);        
        display_column(1, col1,1, col1_2);
        _delay_us(10);        
        display_column(2, col2,2, col2_2);
        _delay_us(10);        
        display_column(3, col3,3, col3_2);
        _delay_us(10);        
        display_column(4, col4,4, col4_2);
        _delay_us(10);        
        display_column(5, col5,5, col5_2);
        _delay_us(10);        
        display_column(6, col6,6, col6_2);
        _delay_us(10);
        display_column(7, col7, 7, col7_2);
        _delay_us(10);
    }

}

void display_number_2( int thousands, int hundreds, int tens, int ones, int time ) { 

    if( thousands < 0 ) { 
        if( hundreds < 0 ) { 
            if ( tens < 0 ) { 
                
                display_pattern( patterns[ones][0] , 
                                 patterns[ones][1] , 
                                 patterns[ones][2] , 
                                 patterns[ones][3] , 
                                 patterns[ones][4] , 
                                 patterns[ones][5] , 
                                 patterns[ones][6] , 
                                 patterns[ones][7] , 
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 time );
            }
            else { 
                display_pattern( patterns[ones][0] | (patterns[tens][0] << 4 ), 
                                 patterns[ones][1] | (patterns[tens][1] << 4 ), 
                                 patterns[ones][2] | (patterns[tens][2] << 4 ), 
                                 patterns[ones][3] | (patterns[tens][3] << 4 ), 
                                 patterns[ones][4] | (patterns[tens][4] << 4 ), 
                                 patterns[ones][5] | (patterns[tens][5] << 4 ), 
                                 patterns[ones][6] | (patterns[tens][6] << 4 ), 
                                 patterns[ones][7] | (patterns[tens][7] << 4 ), 
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 time );
            }
        }
        else { 
            display_pattern( patterns[ones][0] | (patterns[tens][0] << 4 ), 
                             patterns[ones][1] | (patterns[tens][1] << 4 ), 
                             patterns[ones][2] | (patterns[tens][2] << 4 ), 
                             patterns[ones][3] | (patterns[tens][3] << 4 ), 
                             patterns[ones][4] | (patterns[tens][4] << 4 ), 
                             patterns[ones][5] | (patterns[tens][5] << 4 ), 
                             patterns[ones][6] | (patterns[tens][6] << 4 ), 
                             patterns[ones][7] | (patterns[tens][7] << 4 ), 
                             patterns[hundreds][0] , 
                             patterns[hundreds][1] , 
                             patterns[hundreds][2] , 
                             patterns[hundreds][3] , 
                             patterns[hundreds][4] , 
                             patterns[hundreds][5] , 
                             patterns[hundreds][6] , 
                             patterns[hundreds][7] , 
                             time );
        }
    }
    else {
        display_pattern( patterns[ones][0] | (patterns[tens][0] << 4 ), 
                         patterns[ones][1] | (patterns[tens][1] << 4 ), 
                         patterns[ones][2] | (patterns[tens][2] << 4 ), 
                         patterns[ones][3] | (patterns[tens][3] << 4 ), 
                         patterns[ones][4] | (patterns[tens][4] << 4 ), 
                         patterns[ones][5] | (patterns[tens][5] << 4 ), 
                         patterns[ones][6] | (patterns[tens][6] << 4 ), 
                         patterns[ones][7] | (patterns[tens][7] << 4 ), 
                         patterns[hundreds][0] | (patterns[thousands][0] << 4 ), 
                         patterns[hundreds][1] | (patterns[thousands][1] << 4 ), 
                         patterns[hundreds][2] | (patterns[thousands][2] << 4 ), 
                         patterns[hundreds][3] | (patterns[thousands][3] << 4 ), 
                         patterns[hundreds][4] | (patterns[thousands][4] << 4 ), 
                         patterns[hundreds][5] | (patterns[thousands][5] << 4 ), 
                         patterns[hundreds][6] | (patterns[thousands][6] << 4 ), 
                         patterns[hundreds][7] | (patterns[thousands][7] << 4 ), 
                         time );
    }
}

void display_number( int number, int time ) {

    int thousands = number/1000;
    int hundreds = (number-thousands*1000)/100;
    int tens = (number-thousands*1000-hundreds*100)/10;
    int ones = (number-thousands*1000-hundreds*100-tens*10);

    if( thousands == 0 ) {
        thousands = -1;
        if( hundreds == 0) { 
            hundreds = -1;
            if( tens == 0 ) { 
                tens = -1;
            }
        }
    }
    display_number_2( thousands, hundreds, tens, ones, time );

}    
        


//void display_number( int tens, int ones, int time ) { 
//
//    if ( tens < 0 ) { 
//        
//        display_pattern( patterns[ones][0] , 
//                         patterns[ones][1] , 
//                         patterns[ones][2] , 
//                         patterns[ones][3] , 
//                         patterns[ones][4] , 
//                         patterns[ones][5] , 
//                         patterns[ones][6] , 
//                         patterns[ones][7] , 
//                         time );
//    }
//    else { 
//        display_pattern( patterns[ones][0] | (patterns[tens][0] << 4 ), 
//                         patterns[ones][1] | (patterns[tens][1] << 4 ), 
//                         patterns[ones][2] | (patterns[tens][2] << 4 ), 
//                         patterns[ones][3] | (patterns[tens][3] << 4 ), 
//                         patterns[ones][4] | (patterns[tens][4] << 4 ), 
//                         patterns[ones][5] | (patterns[tens][5] << 4 ), 
//                         patterns[ones][6] | (patterns[tens][6] << 4 ), 
//                         patterns[ones][7] | (patterns[tens][7] << 4 ), 
//                         time );
//
//    }
//}

    
int main(void)
{
  DDRB = 0xff;

  // start with reset off
  PORTB = 0x01 << _bit_reset;
  //delay_ms(1000);

  // Reset shift registers
  PORTB = 0x00; 
  // keep the reset active for some time (can be tuned)
  //delay_ms(1000);
  _delay_us(1);
  PORTB = 0x01 << _bit_reset;

  while (1) {

    //display_column(0, 0xff,7, 0x99);
    //

    uint16_t test1 = 0x0000;
    uint8_t test2 = 0x01;
    uint8_t test3 = 0x02;

    test1 = test2;
    test1 = test1 << 8;
    test1 = test1 | test3;

    display_number( test1, 500 );



    //for( int i =0; i < 10000; i++ ) { 
    //    display_number( i, 1);
    //}

      //PORTB = 0x03j
      
    //for( int t = 0; t < 10; t ++ ) { 
    //    int thousands_val = -1;
    //    if( t > 0 ) thousands_val = t;
    //    for( int h = 0; h < 10; h ++ ) { 
    //        int hundreds_val = -1;
    //        if( h > 0 || t > 0 ) hundreds_val = h;
    //        for( int i = 0; i < 10; i ++ ) { 
    //            int tens_val = -1;
    //            if( i > 0 || h > 0  || t > 0 ) tens_val = i;
    //            for( int j = 0; j < 10 ; j++ ) { 
    //                display_number_2( thousands_val, hundreds_val, tens_val, j , 5 );

    //            }
    //        }
    //    }
    //}

  }
}

