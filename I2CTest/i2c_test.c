#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"

int _bit_reset = 3;
int _bit_reddata = 2;
int _bit_reddata_2 = 0;
int _bit_copy = 4;
int _bit_coldata = 6;
int _bit_coldata_2 = 1;
int _bit_enable = 7;

int _data_depth = 8;

uint8_t patterns[10][8] = { 
             { 0x0f, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0f, 0x00 },
             { 0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x06, 0x00 },
             { 0x0f, 0x08, 0x08, 0x0f, 0x01, 0x01, 0x0f, 0x00 },
             { 0x0f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x0f, 0x00 },
             { 0x01, 0x01, 0x01, 0x0f, 0x09, 0x09, 0x09, 0x00 },
             { 0x0f, 0x01, 0x01, 0x0f, 0x08, 0x08, 0x0f, 0x00 },
             { 0x0f, 0x09, 0x09, 0x0f, 0x08, 0x08, 0x0f, 0x00 },
             { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0f, 0x00 },
             { 0x0f, 0x09, 0x09, 0x0f, 0x09, 0x09, 0x0f, 0x00 },
             { 0x01, 0x01, 0x01, 0x0f, 0x09, 0x09, 0x0f, 0x00 }
};

uint8_t slave_address_write = 0xd2;
uint8_t slave_address_read = 0xd3;
uint8_t reg_accel_config = 0x1c;
uint8_t reg_accel_z_H = 0x3f;
uint8_t reg_accel_z_L = 0x40;
uint8_t reg_pwr = 0x6b;
uint8_t reg_who_am_i = 0x75;

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
        _delay_us(50);        
        display_column(0, col0,0, col0_2);
        _delay_us(50);        
        display_column(1, col1,1, col1_2);
        _delay_us(50);        
        display_column(2, col2,2, col2_2);
        _delay_us(50);        
        display_column(3, col3,3, col3_2);
        _delay_us(50);        
        display_column(4, col4,4, col4_2);
        _delay_us(50);        
        display_column(5, col5,5, col5_2);
        _delay_us(50);
        display_column(6, col6,6, col6_2);
        _delay_us(50);
        display_column(7, col7, 7, col7_2);
    }

}

void display_number_split( int thousands, int hundreds, int tens, int ones, int time ) { 

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

void display_number( uint16_t number, int time ) {

    int thousands = number/10000;
    int hundreds = (number-thousands*10000)/1000;
    int tens = (number-thousands*10000-hundreds*1000)/100;
    int ones = (number-thousands*10000-hundreds*1000-tens*100)/10;

    if( thousands == 0 ) {
        thousands = -1;
        if( hundreds == 0) { 
            hundreds = -1;
            if( tens == 0 ) { 
                tens = -1;
            }
        }
    }
    display_number_split( thousands, hundreds, tens, ones, time );

}    

int main( void ) {

    i2c_init();
    DDRB = 0xff;
    PORTB = 0x01 << _bit_reset;
    PORTB = 0x00; 
    _delay_us(1);
    PORTB = 0x01 << _bit_reset;

    i2c_start(slave_address_write);
    i2c_write(reg_pwr); // Config accelerometer
    i2c_write(0x00); // no self-test, finest accel values
    i2c_stop();

    display_number( 11111, 10 );
    uint16_t accel;
    while(1) {

        // read z accel
        if(i2c_start(slave_address_write))  break;
        if(i2c_write( reg_accel_z_H ) ) break;
        //if(i2c_write( reg_who_am_i ) ) break;
        if(i2c_start(slave_address_read)) break;
        accel = i2c_read_nack();
        i2c_stop();
        accel = accel << 8;

        if(i2c_start(slave_address_write) )break;
        if(i2c_write( reg_accel_z_L )) break;
        if(i2c_start(slave_address_read))break;
        accel = accel | i2c_read_nack();
        i2c_stop();

        display_number( accel, 5 );
        
    }
    display_number( 55555, 1000 );


}
