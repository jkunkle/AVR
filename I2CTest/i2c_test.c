#ifndef F_CPU
#define F_CPU 1000000UL // 1 MHz clock speed
#endif
#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"

int _bit_reset = 3;
int _bit_reddata = 4;
int _bit_reddata_2 = 2;
int _bit_reddata_3 = 0;
int _bit_copy = 4;
int _bit_coldata = 5;
int _bit_coldata_2 = 3;
int _bit_coldata_3 = 1;
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

// BMI
uint8_t slave_address_read = 0x33;
uint8_t slave_address_write = 0x32;
uint8_t reg_accel_z_H = 0x07;
uint8_t reg_accel_z_L = 0x06;
uint8_t reg_who_am_i = 0x00;

////MPU
//uint8_t slave_address_write = 0xd2;
//uint8_t slave_address_read = 0xd3;
//uint8_t reg_accel_config = 0x1c;
//uint8_t reg_accel_z_H = 0x3f;
//uint8_t reg_accel_z_L = 0x40;
//uint8_t reg_pwr = 0x6b;
//uint8_t reg_who_am_i = 0x75;

void display_column( int icol, int col_data, int icol2, int col_data2, int icol3, int col_data3 ) { 

  //int col_mapping[8] = {3, 2, 1, 0, 4, 5, 6, 7};
  //int col_mapping[8] = {3, 2, 1, 0, 4, 5, 6, 7};
  int col_mapping[8] = {0, 1, 2, 3, 7, 6, 5, 4};

  // disable output
  PORTB = PORTB | (0x1 << _bit_enable);

  // set the data
  for( int dd =0; dd <= _data_depth; dd++ ) {
  //for( int dd =_data_depth-1; dd > 0; dd-- ) {

      if( col_mapping[dd] == icol ) {
          PORTD = PORTD | (0x1 << _bit_coldata );
      }
      if( col_mapping[dd] == icol2 ) {
          PORTD = PORTD | (0x1 << _bit_coldata_2 );
      }
      if( col_mapping[dd] == icol3 ) {
          PORTD = PORTD | (0x1 << _bit_coldata_3 );
      }
      if( col_data & (0x1 << dd) ) {
          PORTD = PORTD | (0x1 << _bit_reddata ) ;
      }
      if( col_data2 & (0x1 << dd) ) {
          PORTD = PORTD | (0x1 << _bit_reddata_2 ) ;
      }
      if( col_data3 & (0x1 << dd) ) {
          PORTD = PORTD | (0x1 << _bit_reddata_3 ) ;
      }

      PORTB = PORTB | (0x1 << _bit_copy );

      // set data and copy signals to 0
      PORTB = PORTB & ~(0x1 << _bit_copy );
      PORTD = PORTD & ~(0x1 << _bit_coldata );
      PORTD = PORTD & ~(0x1 << _bit_reddata );
      PORTD = PORTD & ~(0x1 << _bit_coldata_2 );
      PORTD = PORTD & ~(0x1 << _bit_reddata_2 );
      PORTD = PORTD & ~(0x1 << _bit_coldata_3 );
      PORTD = PORTD & ~(0x1 << _bit_reddata_3 );
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
                      int col0_3, int col1_3,
                      int col2_3, int col3_3,
                      int col4_3, int col5_3,
                      int col6_3, int col7_3,
                      int time ) {

    for(int i = 0; i < time ; ++i ) {
        _delay_us(50);        
        display_column(0, col0,0, col0_2,0, col0_3);
        _delay_us(50);        
        display_column(1, col1,1, col1_2,1, col1_3);
        _delay_us(50);        
        display_column(2, col2,2, col2_2,2, col2_3);
        _delay_us(50);        
        display_column(3, col3,3, col3_2,3, col3_3);
        _delay_us(50);        
        display_column(4, col4,4, col4_2,4, col4_3);
        _delay_us(50);        
        display_column(5, col5,5, col5_2,5, col5_3);
        _delay_us(50);                  
        display_column(6, col6,6, col6_2,6, col6_3);
        _delay_us(50);                  
        display_column(7, col7,7, col7_2,7, col7_3);
    }

}

void display_number_split( int Tthousands, int thousands, int hundreds, int tens, int ones, int time ) { 

    if( Tthousands < 0 ) { 
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
                    display_pattern( patterns[ones][0] | (patterns[tens][0] << 5 ), 
                                     patterns[ones][1] | (patterns[tens][1] << 5 ), 
                                     patterns[ones][2] | (patterns[tens][2] << 5 ), 
                                     patterns[ones][3] | (patterns[tens][3] << 5 ), 
                                     patterns[ones][4] | (patterns[tens][4] << 5 ), 
                                     patterns[ones][5] | (patterns[tens][5] << 5 ), 
                                     patterns[ones][6] | (patterns[tens][6] << 5 ), 
                                     patterns[ones][7] | (patterns[tens][7] << 5 ), 
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
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
                display_pattern( 
                                 patterns[ones][0] | (patterns[tens][0] << 5 ), 
                                 patterns[ones][1] | (patterns[tens][1] << 5 ), 
                                 patterns[ones][2] | (patterns[tens][2] << 5 ), 
                                 patterns[ones][3] | (patterns[tens][3] << 5 ), 
                                 patterns[ones][4] | (patterns[tens][4] << 5 ), 
                                 patterns[ones][5] | (patterns[tens][5] << 5 ), 
                                 patterns[ones][6] | (patterns[tens][6] << 5 ), 
                                 patterns[ones][7] | (patterns[tens][7] << 5 ), 
                                 ( patterns[hundreds][0] << 2 ) | (patterns[tens][0] >> 3 ), 
                                 ( patterns[hundreds][1] << 2 ) | (patterns[tens][1] >> 3 ), 
                                 ( patterns[hundreds][2] << 2 ) | (patterns[tens][2] >> 3 ), 
                                 ( patterns[hundreds][3] << 2 ) | (patterns[tens][3] >> 3 ), 
                                 ( patterns[hundreds][4] << 2 ) | (patterns[tens][4] >> 3 ), 
                                 ( patterns[hundreds][5] << 2 ) | (patterns[tens][5] >> 3 ), 
                                 ( patterns[hundreds][6] << 2 ) | (patterns[tens][6] >> 3 ), 
                                 ( patterns[hundreds][7] << 2 ) | (patterns[tens][7] >> 3 ), 
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
            display_pattern( 
                              patterns[ones][0] | (patterns[tens][0] << 5 ), 
                              patterns[ones][1] | (patterns[tens][1] << 5 ), 
                              patterns[ones][2] | (patterns[tens][2] << 5 ), 
                              patterns[ones][3] | (patterns[tens][3] << 5 ), 
                              patterns[ones][4] | (patterns[tens][4] << 5 ), 
                              patterns[ones][5] | (patterns[tens][5] << 5 ), 
                              patterns[ones][6] | (patterns[tens][6] << 5 ), 
                              patterns[ones][7] | (patterns[tens][7] << 5 ), 
                             ( patterns[hundreds][0] << 2 ) | (patterns[thousands][0] << 7 ) | (patterns[tens][0] >> 3 ), 
                             ( patterns[hundreds][1] << 2 ) | (patterns[thousands][1] << 7 ) | (patterns[tens][1] >> 3 ), 
                             ( patterns[hundreds][2] << 2 ) | (patterns[thousands][2] << 7 ) | (patterns[tens][2] >> 3 ), 
                             ( patterns[hundreds][3] << 2 ) | (patterns[thousands][3] << 7 ) | (patterns[tens][3] >> 3 ), 
                             ( patterns[hundreds][4] << 2 ) | (patterns[thousands][4] << 7 ) | (patterns[tens][4] >> 3 ), 
                             ( patterns[hundreds][5] << 2 ) | (patterns[thousands][5] << 7 ) | (patterns[tens][5] >> 3 ), 
                             ( patterns[hundreds][6] << 2 ) | (patterns[thousands][6] << 7 ) | (patterns[tens][6] >> 3 ), 
                             ( patterns[hundreds][7] << 2 ) | (patterns[thousands][7] << 7 ) | (patterns[tens][7] >> 3 ), 
                                     (patterns[thousands][0] >> 1 ),
                                     (patterns[thousands][1] >> 1 ),
                                     (patterns[thousands][2] >> 1 ),
                                     (patterns[thousands][3] >> 1 ),
                                     (patterns[thousands][4] >> 1 ),
                                     (patterns[thousands][5] >> 1 ),
                                     (patterns[thousands][6] >> 1 ),
                                     (patterns[thousands][7] >> 1 ),
                             time );
        }
    }
    else { 
        display_pattern( 
                         patterns[ones][0] | (patterns[tens][0] << 5 ), 
                         patterns[ones][1] | (patterns[tens][1] << 5 ), 
                         patterns[ones][2] | (patterns[tens][2] << 5 ), 
                         patterns[ones][3] | (patterns[tens][3] << 5 ), 
                         patterns[ones][4] | (patterns[tens][4] << 5 ), 
                         patterns[ones][5] | (patterns[tens][5] << 5 ), 
                         patterns[ones][6] | (patterns[tens][6] << 5 ), 
                         patterns[ones][7] | (patterns[tens][7] << 5 ), 
                         ( patterns[hundreds][0] << 2 ) | (patterns[thousands][0] << 7 ) | (patterns[tens][0] >> 3 ), 
                         ( patterns[hundreds][1] << 2 ) | (patterns[thousands][1] << 7 ) | (patterns[tens][1] >> 3 ), 
                         ( patterns[hundreds][2] << 2 ) | (patterns[thousands][2] << 7 ) | (patterns[tens][2] >> 3 ), 
                         ( patterns[hundreds][3] << 2 ) | (patterns[thousands][3] << 7 ) | (patterns[tens][3] >> 3 ), 
                         ( patterns[hundreds][4] << 2 ) | (patterns[thousands][4] << 7 ) | (patterns[tens][4] >> 3 ), 
                         ( patterns[hundreds][5] << 2 ) | (patterns[thousands][5] << 7 ) | (patterns[tens][5] >> 3 ), 
                         ( patterns[hundreds][6] << 2 ) | (patterns[thousands][6] << 7 ) | (patterns[tens][6] >> 3 ), 
                         ( patterns[hundreds][7] << 2 ) | (patterns[thousands][7] << 7 ) | (patterns[tens][7] >> 3 ), 
                         ( patterns[Tthousands][0] << 4 ) | (patterns[thousands][0] >> 1 ) , 
                         ( patterns[Tthousands][1] << 4 ) | (patterns[thousands][1] >> 1 ) , 
                         ( patterns[Tthousands][2] << 4 ) | (patterns[thousands][2] >> 1 ) , 
                         ( patterns[Tthousands][3] << 4 ) | (patterns[thousands][3] >> 1 ) , 
                         ( patterns[Tthousands][4] << 4 ) | (patterns[thousands][4] >> 1 ) , 
                         ( patterns[Tthousands][5] << 4 ) | (patterns[thousands][5] >> 1 ) , 
                         ( patterns[Tthousands][6] << 4 ) | (patterns[thousands][6] >> 1 ) , 
                         ( patterns[Tthousands][7] << 4 ) | (patterns[thousands][7] >> 1 ) , 
                         time );
    }

}

void display_number( uint16_t number, int time ) {

    int Tthousands = number/10000;
    int thousands = (number-Tthousands*10000)/1000;
    int hundreds = (number-Tthousands*10000-thousands*1000)/100;
    int tens = (number-Tthousands*10000-thousands*1000-hundreds*100)/10;
    int ones = (number-Tthousands*10000-thousands*1000-hundreds*100-tens*10);

    if( Tthousands == 0 ) {
        Tthousands = -1;
        if( thousands == 0 ) {
            thousands = -1;
            if( hundreds == 0) { 
                hundreds = -1;
                if( tens == 0 ) { 
                    tens = -1;
                }
            }
        }
    }
    display_number_split( Tthousands, thousands, hundreds, tens, ones, time );

}    
uint8_t ReadADC(uint8_t ADCchannel)
{
 //select ADC channel with safety mask
 ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
 //single conversion mode
 ADCSRA |= (1<<ADSC);
 // wait until ADC conversion is complete
 while( ADCSRA & (1<<ADSC) );
 return ADC;
}

int main( void ) {

    i2c_init();
    DDRB = 0xff;
    DDRD = 0xff;
    PORTB = 0x00; 
    _delay_us(100);
    PORTB = 0x01 << _bit_reset;

    _delay_us(100);
    ADMUX |= ( 1 << REFS1 ) | ( 1 << REFS0 ); //use internal refernce (with cap on AREF)
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); // set prescale to 128 and enable ADC
    _delay_us(100);

    //// MPU
    //// comment out for BMI
    //i2c_start(slave_address_write);
    //i2c_write(reg_pwr); // Config accelerometer
    //i2c_write(0x00); // no self-test, finest accel values
    //i2c_stop();


    display_number( 11111, 100 );
    uint16_t accel;
    uint8_t accel_l;
    while(1) {

        display_number( ReadADC(0), 100 );

        //// MPU
        //// read z accel
        //if(i2c_start(slave_address_write))  break;
        //if(i2c_write( reg_accel_z_H ) ) break;
        ////if(i2c_write( reg_who_am_i ) ) break;
        //if(i2c_start(slave_address_read)) break;
        //accel = i2c_read_nack();
        //i2c_stop();
        //accel = accel << 8;

        //if(i2c_start(slave_address_write) )break;
        //if(i2c_write( reg_accel_z_L )) break;
        //if(i2c_start(slave_address_read))break;
        //accel = accel | i2c_read_nack();
        //i2c_stop();

        /////BMI
        ///// read z accel
        ///// LSB 
        ///if(i2c_start(slave_address_write) )break;
        ///if(i2c_write( reg_accel_z_L )) break;
        ///if(i2c_start(slave_address_read))break;
        ///accel_l = i2c_read_nack();
        ///i2c_stop();

        /////msb
        ///if(i2c_start(slave_address_write))  break;
        ///if(i2c_write( reg_accel_z_H ) ) break;
        /////if(i2c_write( reg_who_am_i ) ) break;
        ///if(i2c_start(slave_address_read)) break;
        ///accel = i2c_read_nack();
        ///i2c_stop();
        ///accel = accel << 4;
        ///accel = accel | ( accel_l >> 4 );

        ///display_number( accel, 5 );
        
    }
    display_number( 55555, 1000 );


}
