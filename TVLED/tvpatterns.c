/*
* Light_WS2812 library example - RGB_blinky
*
* cycles one LED through red, green, blue
*
* This example is configured for a ATtiny85 with PLL clock fuse set and
* the WS2812 string connected to PB4.
*/

// TVCOLORS 
// VIOLET : #8B3E6E RED=16, BLUE=2-4
// CYAN : #4DBAC3 BLUE=16 GREEN=12-16
// DIRTY YELLOW : #C6C474 RED=16 GREEN=5-6
// #682D55
// EGGSHELL WHITE : #E3E1CD: RED=16, GREEN=6 BLUE=1
// #333333
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "light_ws2812.h"

#define _N_LED_VIOLET 170
#define _N_LED_BEIGE 83
#define _N_LED_YELLOW 116
#define _N_LED_CYAN 170
#define _MAX_LED _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW + _N_LED_CYAN + 1
#define _MAX_DELAY 1024
#define _MIN_DELAY 32
#define _N_PAT 7
//#define _N_RACE_PAT 41 
#define _N_RACE_PAT 4 
#define _N_DISCO_PAT 50

#define _START_VIOLET 0
#define _START_BEIGE _N_LED_VIOLET
#define _START_YELLOW _N_LED_VIOLET + _N_LED_BEIGE
#define _START_CYAN _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW

FUSES = 
{
    ///.low = (FUSE_CKSEL0 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0 ), // E2
    .low = 0xff, 
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};


struct cRGB led[_MAX_LED];


int BUTTON_VETO = 0;

const int violet[4] = {8, 0, 1, 1};
const int cyan[4] = {0, 3, 4, 1};
const int yellow[4] = {8, 3, 0, 1};
const int beige[4] = {8, 3, 1, 1};

const int max_delays[_N_PAT] = {16,2048, 256, 256, 1024, 256, 1024};
const int min_delays[_N_PAT] = {16,8, 1, 1, 1, 4, 1024};

volatile int DELAY = max_delays[0];

int patterns_race[_N_RACE_PAT][3] = { 
    
                                      {0, 39, 81},
                                      {1, 39, 81},
                                      {2, 39, 81},
                                      {2, 39, 66}, 
                                      {3, 40, 67}, 
                                      {4, 41, 68}, 
                                      {5, 42, 69}, 
                                      {6, 43, 70}, 
                                      {7, 44, 71}, 
                                      {8, 45, 72}, 
                                      {9, 46, 72}, 
                                      {10, 47, 72}, 
                                      {11, 48, 72}, 
                                      {12, 49, 72},
                                      {13, 49, 72},
                                      {14, 49, 72},
                                      {15, 50, 73},
                                      {16, 50, 73},
                                      {17, 50, 73},
                                      {18, 51, 73},
                                      {19, 52, 73},
                                      {20, 53, 73},
                                      {21, 54, 73},
                                      {22, 55, 73},
                                      {23, 56, 74},
                                      {24, 57, 75},
                                      {25, 58, 76},
                                      {26, 59, 77},
                                      {27, 60, 78},
                                      {28, 61, 79},
                                      {29, 62, 80},
                                      {30, 62, 80},
                                      {31, 62, 80},
                                      {32, 62, 80},
                                      {33, 62, 80},
                                      {34, 63, 80},
                                      {35, 64, 81},
                                      {36, 65, 81},
                                      {37, 66, 81},
                                      {38, 67, 81}
                            };

volatile int ipat = 0;
volatile int istep = 0;
volatile int isub = 0;
volatile int stop_updates = 0;
volatile int idirection = 1;

uint8_t TCCR1B_SEL = (1 << CS11 ) | (1 << CS10 );


ISR( TIMER1_OVF_vect ) {
    
    BUTTON_VETO = 0;
    TCCR1B &= ~TCCR1B_SEL;

}

ISR(INT0_vect)
{
    if (BUTTON_VETO == 0 ) { 

        ipat += 1;
        istep = 0;
        if( ipat >= (_N_PAT-1) ) { 
            ipat = 0;
        }
        DELAY = max_delays[ipat];
        stop_updates = 0;


        for( int il = 0 ; il < _MAX_LED; il++ ) {
            led[il].r=0;
            led[il].g=0;
            led[il].b=0;
        }
        ws2812_setleds(led,_MAX_LED);

        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

ISR(INT1_vect)
{
    if (BUTTON_VETO == 0 ) { 
        DELAY /= 2;
        if( DELAY <= min_delays[ipat]) DELAY=max_delays[ipat];
        
        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

uint8_t random( uint8_t seed ) { 

    seed ^= seed << 4;
    seed ^= seed >> 7;
    seed ^= seed << 1;

    return seed;

}

uint8_t fill_random( struct cRGB *led, int brightness, uint8_t seed ) {

     uint8_t rand_violet = random( seed );
     uint8_t rand_beige = random( rand_violet );
     uint8_t rand_yellow = random( rand_beige );
     uint8_t rand_cyan = random( rand_yellow );

     if( rand_violet >= _N_LED_VIOLET ) {
         rand_violet -= _N_LED_VIOLET;
     }

     // only need 7 bits (128) for beige/yellow
     rand_beige = rand_beige >> 1;
     if( rand_beige > _N_LED_BEIGE ) { 
         rand_beige -= _N_LED_BEIGE;
     }

     rand_yellow = rand_yellow >> 1;
     if( rand_yellow > _N_LED_YELLOW) { 
         rand_yellow -= _N_LED_YELLOW;
     }

     if( rand_cyan >= _N_LED_CYAN ) { 
         rand_cyan -= _N_LED_CYAN;
     }

     uint16_t val_beige = rand_beige + _START_BEIGE;

     uint16_t val_yellow = rand_yellow + _START_YELLOW;

     uint16_t val_cyan = rand_cyan + _START_CYAN;

     led[rand_violet].r = violet[0]*brightness;
     led[rand_violet].g = violet[1]*brightness;
     led[rand_violet].b = violet[2]*brightness;

     led[val_beige].r  = beige[0]*brightness;
     led[val_beige].g  = beige[1]*brightness;
     led[val_beige].b  = beige[2]*brightness;

     led[val_yellow].r  = yellow[0]*brightness;
     led[val_yellow].g  = yellow[1]*brightness;
     led[val_yellow].b  = yellow[2]*brightness;

     led[val_cyan].r  = cyan[0]*brightness;
     led[val_cyan].g  = cyan[1]*brightness;
     led[val_cyan].b  = cyan[2]*brightness;

     return rand_cyan;
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

int main(void)
{
  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1<<PD2 ) | (1 << PD3 );   // enable PORTD.2, PORTD.3 pin pull up resistor
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge

  ADMUX |= ( 1 << REFS1 ) | ( 1 << REFS0 ); //use internal refernce (with cap on AREF)
  ADMUX |= ( 1 << ADLAR );

  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); // set prescale to 128 and enable ADC

  sei();


  _delay_us(100);
  while(1) {

      if( ipat == 0 ) { 
          if( istep > 15 ) { 
              stop_updates = 1;
          }
          if( stop_updates == 1 ) { 
              continue;
          }
          for( int il = _START_VIOLET ; il < _START_BEIGE; il++ ) {
              led[il].r=violet[0]*istep;
              led[il].g=violet[1]*istep;
              led[il].b=violet[2]*istep;
          }
          for( int il = _START_BEIGE ; il < _START_YELLOW; il++ ) {
              led[il].r=beige[0]*istep;
              led[il].g=beige[1]*istep;
              led[il].b=beige[2]*istep;
          }
          for( int il = _START_YELLOW ; il < _START_CYAN; il++ ) {
              led[il].r=yellow[0]*istep;
              led[il].g=yellow[1]*istep;
              led[il].b=yellow[2]*istep;
          }
          for( int il = _START_CYAN ; il < _MAX_LED; il++ ) {
              led[il].r=cyan[0]*istep;
              led[il].g=cyan[1]*istep;
              led[il].b=cyan[2]*istep;
          }
          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
      }
          
      if( ipat == 1 ) { 
          if( istep > 2 ) { 
              istep = 0;
          }
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }
          if( istep == 0 ) { 
              for( int il = 0 ; il < 11; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 53 ; il < 70; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 74 ; il < 90; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 237 ; il < 253; il++ ) {
                  led[il].r=beige[0]*4;
                  led[il].g=beige[1]*4;
                  led[il].b=beige[2]*4;
              }
              for( int il = 347 ; il < 369; il++ ) {
                  led[il].r=yellow[0]*4;
                  led[il].g=yellow[1]*4;
                  led[il].b=yellow[2]*4;
              }
              for( int il = 420 ; il < 437; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 481 ; il < 498; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 529 ; il < _MAX_LED; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }

          }
          else if( istep == 1) {
              led[11].r=violet[0]*4;
              led[11].g=violet[1]*4;
              led[11].b=violet[2]*4;

              led[72].r=violet[0]*4;
              led[72].g=violet[1]*4;
              led[72].b=violet[2]*4;

              led[90].r=violet[0]*4;
              led[90].g=violet[1]*4;
              led[90].b=violet[2]*4;

              led[437].r=cyan[0]*4;
              led[437].g=cyan[1]*4;
              led[437].b=cyan[2]*4;

              led[419].r=cyan[0]*4;
              led[419].g=cyan[1]*4;
              led[419].b=cyan[2]*4;

              led[528].r=cyan[0]*4;
              led[528].g=cyan[1]*4;
              led[528].b=cyan[2]*4;

              for( int il = 34 ; il < 53; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 92 ; il < 105; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 122 ; il < 142; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 209 ; il < 237; il++ ) {
                  led[il].r=beige[0]*4;
                  led[il].g=beige[1]*4;
                  led[il].b=beige[2]*4;
              }
              for( int il = 307 ; il < 347; il++ ) {
                  led[il].r=yellow[0]*4;
                  led[il].g=yellow[1]*4;
                  led[il].b=yellow[2]*4;
              }
              for( int il = 397 ; il < 417; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 462 ; il < 481; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 513 ; il < 527; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
          }
          else if( istep == 2) {
              led[12].r=violet[0]*4;
              led[12].g=violet[1]*4;
              led[12].b=violet[2]*4;

              led[70].r=violet[0]*4;
              led[70].g=violet[1]*4;
              led[70].b=violet[2]*4;

              led[91].r=violet[0]*4;
              led[91].g=violet[1]*4;
              led[91].b=violet[2]*4;

              led[110].r=violet[0]*4;
              led[110].g=violet[1]*4;
              led[110].b=violet[2]*4;

              led[418].r=cyan[0]*4;
              led[418].g=cyan[1]*4;
              led[418].b=cyan[2]*4;

              led[417].r=cyan[0]*4;
              led[417].g=cyan[1]*4;
              led[417].b=cyan[2]*4;
              
              led[440].r=cyan[0]*4;
              led[440].g=cyan[1]*4;
              led[440].b=cyan[2]*4;

              led[394].r=cyan[0]*4;
              led[394].g=cyan[1]*4;
              led[394].b=cyan[2]*4;

              led[512].r=cyan[0]*4;
              led[512].g=cyan[1]*4;
              led[512].b=cyan[2]*4;

              led[527].r=cyan[0]*4;
              led[527].g=cyan[1]*4;
              led[527].b=cyan[2]*4;

              led[526].r=cyan[0]*4;
              led[526].g=cyan[1]*4;
              led[526].b=cyan[2]*4;

              for( int il = 13 ; il < 34; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 105 ; il < 122; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 144 ; il < 170; il++ ) {
                  led[il].r=violet[0]*4;
                  led[il].g=violet[1]*4;
                  led[il].b=violet[2]*4;
              }
              for( int il = 170 ; il < 209; il++ ) {
                  led[il].r=beige[0]*4;
                  led[il].g=beige[1]*4;
                  led[il].b=beige[2]*4;
              }
              for( int il = 253 ; il < 307; il++ ) {
                  led[il].r=yellow[0]*4;
                  led[il].g=yellow[1]*4;
                  led[il].b=yellow[2]*4;
              }
              for( int il = 369 ; il < 394; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 441 ; il < 462; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
              for( int il = 498 ; il < 513; il++ ) {
                  led[il].r=cyan[0]*4;
                  led[il].g=cyan[1]*4;
                  led[il].b=cyan[2]*4;
              }
          }
          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.

      }
      else if( ipat == 2 ) { 
          if( istep >= _MAX_LED ) { 
              istep = 0;
          }
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }
          if( istep < _START_BEIGE  ) { 
             led[istep].r=violet[0]*4;
             led[istep].g=violet[1]*4;
             led[istep].b=violet[2]*4;
          }
          if( istep >= _START_BEIGE && istep < _START_YELLOW  ) { 
             led[istep].r=beige[0]*4;
             led[istep].g=beige[1]*4;
             led[istep].b=beige[2]*4;
          }
          if( istep >= _START_YELLOW && istep < _START_CYAN  ) { 
             led[istep].r=yellow[0]*4;
             led[istep].g=yellow[1]*4;
             led[istep].b=yellow[2]*4;
          }
          if( istep >= _START_CYAN && istep < _MAX_LED ) { 
             led[istep].r=cyan[0]*4;
             led[istep].g=cyan[1]*4;
             led[istep].b=cyan[2]*4;
          }
          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
      }
      //breathe
      else if( ipat == 3 ) { 
          if( istep >= 20 ) { 
              istep = 0;
              if( idirection == 0 ) {
                  idirection = 1 ;
              }
              else {
                  idirection = 0 ;
              }
          }

          if( idirection == 0 ) {
              if( istep <= 10 ) {
                  isub = istep;
              }
              else {
                  isub = ( istep - 10 )/2 + 10;
              }
              

              for( int il = _START_VIOLET ; il < _START_BEIGE; il++ ) {
                  led[il].r=violet[0]*(14-isub);
                  led[il].g=violet[1]*(14-isub);
                  led[il].b=violet[2]*(14-isub);
              }
              for( int il = _START_BEIGE; il < _START_YELLOW; il++ ) {
                  led[il].r=beige[0]*(14-isub);
                  led[il].g=beige[1]*(14-isub);
                  led[il].b=beige[2]*(14-isub);
              }
              for( int il = _START_YELLOW; il < _START_CYAN; il++ ) {
                  led[il].r=yellow[0]*(14-isub);
                  led[il].g=yellow[1]*(14-isub);
                  led[il].b=yellow[2]*(14-isub);
              }
              for( int il = _START_CYAN; il < _MAX_LED; il++ ) {
                  led[il].r=cyan[0]*(14-isub);
                  led[il].g=cyan[1]*(14-isub);
                  led[il].b=cyan[2]*(14-isub);
              }
          } 
          else {
              if( istep >= 5 ) {
                  isub = istep;
              }
              else {
                  isub = ( istep )/2;
              }
              for( int il = _START_VIOLET ; il < _START_BEIGE; il++ ) {
                  led[il].r=violet[0]*(isub+1);
                  led[il].g=violet[1]*(isub+1);
                  led[il].b=violet[2]*(isub+1);
              }
              for( int il = _START_BEIGE; il < _START_YELLOW; il++ ) {
                  led[il].r=beige[0]*(isub+1);
                  led[il].g=beige[1]*(isub+1);
                  led[il].b=beige[2]*(isub+1);
              }
              for( int il = _START_YELLOW; il < _START_CYAN; il++ ) {
                  led[il].r=yellow[0]*(isub+1);
                  led[il].g=yellow[1]*(isub+1);
                  led[il].b=yellow[2]*(isub+1);
              }
              for( int il = _START_CYAN; il < _MAX_LED; il++ ) {
                  led[il].r=cyan[0]*(isub+1);
                  led[il].g=cyan[1]*(isub+1);
                  led[il].b=cyan[2]*(isub+1);
              }
          }
          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
      }
      else if( ipat == 4 ) { 
          if( istep >= _N_RACE_PAT ) { 
              istep = 0;
              for( int il = 0 ; il < _MAX_LED; il++ ) {
                  led[il].r=0;
                  led[il].g=0;
                  led[il].b=0;
              }
              ws2812_setleds(led,_MAX_LED);
              _delay_ms(DELAY);                         // wait for 500ms.
          }
          led[patterns_race[istep][0]].r=violet[0]*4;
          led[patterns_race[istep][0]].g=violet[1]*4;
          led[patterns_race[istep][0]].b=violet[2]*4;

          led[patterns_race[istep][1]].r=violet[0]*4;
          led[patterns_race[istep][1]].g=violet[1]*4;
          led[patterns_race[istep][1]].b=violet[2]*4;

          led[patterns_race[istep][2]].r=violet[0]*4;
          led[patterns_race[istep][2]].g=violet[1]*4;
          led[patterns_race[istep][2]].b=violet[2]*4;

          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
      }
      else if( ipat ==  5 ) { 
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }

          uint8_t new_seed = fill_random( led, 16, (uint8_t)istep );
          new_seed = fill_random( led, 16, new_seed );
          new_seed = fill_random( led, 16, new_seed );
          new_seed = fill_random( led, 16, new_seed );
          new_seed = fill_random( led, 16, new_seed );
          fill_random( led, 16, new_seed );

          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
      }
      else if( ipat ==  6 ) { 
        uint8_t adc_val = ReadADC(0);

        for( int il = 0 ; il < _MAX_LED; il++ ) {
            led[il].r=0;
            led[il].g=0;
            led[il].b=0;
        }
        for( int il = 0 ; il < adc_val; il++ ) {
            led[il].r=violet[0];
            led[il].g=violet[1];
            led[il].b=violet[2];
        }

        ws2812_setleds(led,_MAX_LED);
        _delay_ms(DELAY);                         // wait for 500ms.


      }


      istep++;
  }

}
