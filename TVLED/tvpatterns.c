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
#include <avr/pgmspace.h>
#include "light_ws2812.h"

#define _N_LED_VIOLET 170
#define _N_LED_BEIGE 83
#define _N_LED_YELLOW 116
#define _N_LED_CYAN 170
#define _MAX_LED _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW + _N_LED_CYAN + 1
#define _MAX_DELAY 1024
#define _MIN_DELAY 32
#define _N_PAT 8
#define _N_RACE_PAT 50
#define _N_DISCO_PAT 50

#define _START_VIOLET 0
#define _START_BEIGE _N_LED_VIOLET
#define _START_YELLOW _N_LED_VIOLET + _N_LED_BEIGE
#define _START_CYAN _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW

#define SPI_DDR DDRB
#define CS      PB2
#define MOSI    PB3
#define MISO    PB4
#define SCK     PB5

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

const int max_delays[_N_PAT] = {16,256, 256, 256, 256, 256, 1024, 1024};
const int min_delays[_N_PAT] = {16,1, 1, 1, 1, 4, 1024, 1};

volatile int DELAY = max_delays[0];

const uint16_t patterns_race[_N_RACE_PAT][12] PROGMEM = {
                {0, 92, 120, 172, 210, 237, 255, 308, 347, 539, 513, 512},
                {1, 93, 119, 172, 210, 237, 256, 309, 348, 538, 514, 511},
                {2, 94, 118, 173, 211, 238, 257, 310, 349, 537, 515, 510},
                {3, 95, 117, 173, 211, 238, 258, 311, 350, 536, 516, 509},
                {4, 96, 116, 174, 212, 239, 259, 312, 351, 535, 517, 508},
                {5, 97, 115, 174, 212, 239, 260, 313, 352, 534, 518, 507},
                {6, 98, 114, 175, 213, 240, 261, 314, 353, 533, 519, 506},
                {7, 99, 113, 175, 213, 240, 262, 315, 354, 532, 520, 505},
                {8, 100, 112, 176, 214, 241, 263, 316, 355, 531, 521,504},
                {9, 101, 111, 177, 215, 241, 264, 317, 356, 530, 522, 503},
                // corner
                {9, 102, 110, 178, 216, 241, 265, 318, 356, 530, 523, 502},
                {9, 102, 109, 179, 217, 241, 266, 319, 356, 530, 523, 501},
                {10, 103, 108, 180, 218, 242, 267, 320, 356, 529, 524, 500},
                {10, 104, 107, 181, 219, 242, 268, 321, 356, 529, 525, 499},
                {10, 105, 106, 182, 220, 243, 269, 322, 356, 529, 525, 498},
                {10, 11, 12, 183, 221, 243, 270, 323, 356, 529, 528, 527},
                // together again
                {53, 51, 15, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {54, 50, 16, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {55, 49, 17, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {56, 48, 18, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {57, 47, 19, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {58, 46, 20, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {59, 45, 21, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {60, 44, 22, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {61, 43, 23, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {62, 42, 24, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {63, 41, 25, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {64, 40, 26, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {65, 39, 27, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {66, 38, 28, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {67, 37, 29, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {68, 36, 30, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {69, 35, 31, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {69, 34, 32, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {69, 34, 33, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                // together again
                {74, 138, 152, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {75, 137, 153, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {76, 136, 154, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {77, 135, 155, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {78, 134, 156, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {79, 133, 157, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {80, 132, 158, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {81, 131, 159, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {82, 130, 160, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {83, 129, 161, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {84, 128, 162, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {85, 127, 163, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {86, 126, 164, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {87, 125, 165, 178, 216, 243, 276, 328, 368, 530, 533, 492},
                {88, 124, 166, 178, 216, 243, 276, 328, 368, 530, 533, 492}
                            };

volatile int ipat = 0;
volatile uint16_t istep = 0;
volatile int isub = 0;
volatile int stop_updates = 0;
volatile int idirection = 1;
volatile uint8_t prev_value = 0;
volatile uint8_t brightness = 4;

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
        if( ipat >= _N_PAT ) { 
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

ISR(PCINT2_vect){

    if (BUTTON_VETO == 0 ) { 

        brightness = brightness - 1;
        if( brightness <= 0 ) {
            brightness = 5;
        }

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

     uint16_t val_beige = rand_beige + _START_BEIGE - 1;

     uint16_t val_yellow = rand_yellow + _START_YELLOW - 1;

     uint16_t val_cyan = rand_cyan + _START_CYAN - 1;

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

void SPI_init()
{
    // set CS, MOSI and SCK to output
    SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // enable SPI, set as master, and clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);

    PORTB |= (1 << CS);
}

uint8_t SPI_masterReceive()
{
    // transmit dummy byte
    SPDR = 0xFF;

    // Wait for reception complete
    while(!(SPSR & (1 << SPIF)));

    // return Data Register
    return SPDR;
}

int main(void)
{
  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1<<PD2 ) | (1 << PD3 ) | (1 << PD4 );   // enable PORTD.2, PORTD.3, PORTD.4 pin pull up resistor
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge
  // enable interrupt on PCINT20
  PCICR |= (1 << PCIE2);
  PCMSK2 = 0;
  PCMSK2 |= (1 << PCINT20);

  ADMUX |= ( 1 << REFS1 ) | ( 1 << REFS0 ); //use internal refernce (with cap on AREF)
  //ADMUX |= ( 1 << REFS0 ); //VCC reference (with cap on AREF)
  //ADMUX |= ( 1 << ADLAR );

  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); // set prescale to 128 and enable ADC

  SPI_init();
        
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

          if( istep/DELAY >= 3 ) { 
              istep = 0;
          }
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }
          if( istep/DELAY == 0 ) { 
              for( int il = 0 ; il < 11; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 53 ; il < 70; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 74 ; il < 90; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 237 ; il < 253; il++ ) {
                  led[il].r=beige[0]*brightness;
                  led[il].g=beige[1]*brightness;
                  led[il].b=beige[2]*brightness;
              }
              for( int il = 347 ; il < 369; il++ ) {
                  led[il].r=yellow[0]*brightness;
                  led[il].g=yellow[1]*brightness;
                  led[il].b=yellow[2]*brightness;
              }
              for( int il = 420 ; il < 437; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 481 ; il < 498; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 529 ; il < _MAX_LED; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }

          }
          else if( istep/DELAY == 1) {
              led[11].r=violet[0]*brightness;
              led[11].g=violet[1]*brightness;
              led[11].b=violet[2]*brightness;

              led[72].r=violet[0]*brightness;
              led[72].g=violet[1]*brightness;
              led[72].b=violet[2]*brightness;

              led[90].r=violet[0]*brightness;
              led[90].g=violet[1]*brightness;
              led[90].b=violet[2]*brightness;

              led[437].r=cyan[0]*brightness;
              led[437].g=cyan[1]*brightness;
              led[437].b=cyan[2]*brightness;

              led[419].r=cyan[0]*brightness;
              led[419].g=cyan[1]*brightness;
              led[419].b=cyan[2]*brightness;

              led[528].r=cyan[0]*brightness;
              led[528].g=cyan[1]*brightness;
              led[528].b=cyan[2]*brightness;

              for( int il = 34 ; il < 53; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 92 ; il < 105; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 122 ; il < 142; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 209 ; il < 237; il++ ) {
                  led[il].r=beige[0]*brightness;
                  led[il].g=beige[1]*brightness;
                  led[il].b=beige[2]*brightness;
              }
              for( int il = 307 ; il < 347; il++ ) {
                  led[il].r=yellow[0]*brightness;
                  led[il].g=yellow[1]*brightness;
                  led[il].b=yellow[2]*brightness;
              }
              for( int il = 397 ; il < 417; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 462 ; il < 481; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 513 ; il < 527; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
          }
          else if( istep/DELAY == 2) {
              led[12].r=violet[0]*brightness;
              led[12].g=violet[1]*brightness;
              led[12].b=violet[2]*brightness;

              led[70].r=violet[0]*brightness;
              led[70].g=violet[1]*brightness;
              led[70].b=violet[2]*brightness;

              led[91].r=violet[0]*brightness;
              led[91].g=violet[1]*brightness;
              led[91].b=violet[2]*brightness;

              led[110].r=violet[0]*brightness;
              led[110].g=violet[1]*brightness;
              led[110].b=violet[2]*brightness;

              led[418].r=cyan[0]*brightness;
              led[418].g=cyan[1]*brightness;
              led[418].b=cyan[2]*brightness;

              led[417].r=cyan[0]*brightness;
              led[417].g=cyan[1]*brightness;
              led[417].b=cyan[2]*brightness;
              
              led[440].r=cyan[0]*brightness;
              led[440].g=cyan[1]*brightness;
              led[440].b=cyan[2]*brightness;

              led[394].r=cyan[0]*brightness;
              led[394].g=cyan[1]*brightness;
              led[394].b=cyan[2]*brightness;

              led[512].r=cyan[0]*brightness;
              led[512].g=cyan[1]*brightness;
              led[512].b=cyan[2]*brightness;

              led[527].r=cyan[0]*brightness;
              led[527].g=cyan[1]*brightness;
              led[527].b=cyan[2]*brightness;

              led[526].r=cyan[0]*brightness;
              led[526].g=cyan[1]*brightness;
              led[526].b=cyan[2]*brightness;

              for( int il = 13 ; il < 34; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 105 ; il < 122; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 144 ; il < 170; il++ ) {
                  led[il].r=violet[0]*brightness;
                  led[il].g=violet[1]*brightness;
                  led[il].b=violet[2]*brightness;
              }
              for( int il = 170 ; il < 209; il++ ) {
                  led[il].r=beige[0]*brightness;
                  led[il].g=beige[1]*brightness;
                  led[il].b=beige[2]*brightness;
              }
              for( int il = 253 ; il < 307; il++ ) {
                  led[il].r=yellow[0]*brightness;
                  led[il].g=yellow[1]*brightness;
                  led[il].b=yellow[2]*brightness;
              }
              for( int il = 369 ; il < 394; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 441 ; il < 462; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
              for( int il = 498 ; il < 513; il++ ) {
                  led[il].r=cyan[0]*brightness;
                  led[il].g=cyan[1]*brightness;
                  led[il].b=cyan[2]*brightness;
              }
          }
          ws2812_setleds(led,_MAX_LED);
          //_delay_ms(DELAY);                         // wait for 500ms.

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
             led[istep].r=violet[0]*brightness;
             led[istep].g=violet[1]*brightness;
             led[istep].b=violet[2]*brightness;
          }
          if( istep >= _START_BEIGE && istep < _START_YELLOW  ) { 
             led[istep].r=beige[0]*brightness;
             led[istep].g=beige[1]*brightness;
             led[istep].b=beige[2]*brightness;
          }
          if( istep >= _START_YELLOW && istep < _START_CYAN  ) { 
             led[istep].r=yellow[0]*brightness;
             led[istep].g=yellow[1]*brightness;
             led[istep].b=yellow[2]*brightness;
          }
          if( istep >= _START_CYAN && istep < _MAX_LED ) { 
             led[istep].r=cyan[0]*brightness;
             led[istep].g=cyan[1]*brightness;
             led[istep].b=cyan[2]*brightness;
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
      //race track
      else if( ipat == 4 ) { 
          int raceStep = istep/DELAY;
          if( raceStep >= _N_RACE_PAT ) { 
              istep = 0;
              for( int il = 0 ; il < _MAX_LED; il++ ) {
                  led[il].r=0;
                  led[il].g=0;
                  led[il].b=0;
              }
              ws2812_setleds(led,_MAX_LED);
              _delay_ms(DELAY);                         // wait for 500ms.
          }
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }

          uint16_t violet0 = pgm_read_word(&(patterns_race[raceStep][0]));
          uint16_t violet1 = pgm_read_word(&(patterns_race[raceStep][1]));
          uint16_t violet2 = pgm_read_word(&(patterns_race[raceStep][2]));

          uint16_t beige0 = pgm_read_word(&(patterns_race[raceStep][3]));
          uint16_t beige1 = pgm_read_word(&(patterns_race[raceStep][4]));
          uint16_t beige2 = pgm_read_word(&(patterns_race[raceStep][5]));

          uint16_t yellow0 = pgm_read_word(&(patterns_race[raceStep][6]));
          uint16_t yellow1 = pgm_read_word(&(patterns_race[raceStep][7]));
          uint16_t yellow2 = pgm_read_word(&(patterns_race[raceStep][8]));

          uint16_t cyan0 = pgm_read_word(&(patterns_race[raceStep][9]));
          uint16_t cyan1 = pgm_read_word(&(patterns_race[raceStep][10]));
          uint16_t cyan2 = pgm_read_word(&(patterns_race[raceStep][11]));

          led[violet0].r=violet[0]*brightness;
          led[violet0].g=violet[1]*brightness;
          led[violet0].b=violet[2]*brightness;

          led[violet1].r=violet[0]*brightness;
          led[violet1].g=violet[1]*brightness;
          led[violet1].b=violet[2]*brightness;

          led[violet2].r=violet[0]*brightness;
          led[violet2].g=violet[1]*brightness;
          led[violet2].b=violet[2]*brightness;

          led[beige0].r=beige[0]*brightness;
          led[beige0].g=beige[1]*brightness;
          led[beige0].b=beige[2]*brightness;

          led[beige1].r=beige[0]*brightness;
          led[beige1].g=beige[1]*brightness;
          led[beige1].b=beige[2]*brightness;

          led[beige2].r=beige[0]*brightness;
          led[beige2].g=beige[1]*brightness;
          led[beige2].b=beige[2]*brightness;

          led[yellow0].r=yellow[0]*brightness;
          led[yellow0].g=yellow[1]*brightness;
          led[yellow0].b=yellow[2]*brightness;

          led[yellow1].r=yellow[0]*brightness;
          led[yellow1].g=yellow[1]*brightness;
          led[yellow1].b=yellow[2]*brightness;

          led[yellow2].r=yellow[0]*brightness;
          led[yellow2].g=yellow[1]*brightness;
          led[yellow2].b=yellow[2]*brightness;

          led[cyan0].r=cyan[0]*brightness;
          led[cyan0].g=cyan[1]*brightness;
          led[cyan0].b=cyan[2]*brightness;

          led[cyan1].r=cyan[0]*brightness;
          led[cyan1].g=cyan[1]*brightness;
          led[cyan1].b=cyan[2]*brightness;

          led[cyan2].r=cyan[0]*brightness;
          led[cyan2].g=cyan[1]*brightness;
          led[cyan2].b=cyan[2]*brightness;

          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);
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
        uint8_t max_adc = 0;

        for( int i = 0; i < 200; i++){
            uint8_t adc_val = ReadADC(0);
            if( adc_val > max_adc )
            {
                max_adc = adc_val;
            }
        }

        uint8_t brightness = max_adc/10;
        if( brightness < prev_value ) { 
            if( prev_value < 2 ) {
                prev_value = 0;
            }else {
                prev_value -= 2;
            }
            brightness = prev_value;
            //if( brightness > 100 ) brightness = 0;
        }
        else { 
            prev_value = brightness;
        }
        for( int il = 0 ; il < _MAX_LED; ++il) {
             led[il].r=0;
             led[il].g=0;
             led[il].b=0;
        }
        //for( int il = 0 ; il < brightness; ++il) {
        //     led[il].r=violet[0];
        //     led[il].g=violet[1];
        //     led[il].b=violet[2];
        //}
        for( int il = 0 ; il < prev_value; il++ ) {
             led[il].r=violet[0];
             led[il].g=violet[1];
             led[il].b=violet[2];
        }

        //for( int il = 0 ; il < _MAX_LED; il++ ) {
        //  if( il < _START_BEIGE  ) { 
        //     led[il].r=violet[0]*brightness;
        //     led[il].g=violet[1]*brightness;
        //     led[il].b=violet[2]*brightness;
        //  }
        //  if( il >= _START_BEIGE && il < _START_YELLOW  ) { 
        //     led[il].r=beige[0]*brightness;
        //     led[il].g=beige[1]*brightness;
        //     led[il].b=beige[2]*brightness;
        //  }
        //  if( il >= _START_YELLOW && il < _START_CYAN  ) { 
        //     led[il].r=yellow[0]*brightness;
        //     led[il].g=yellow[1]*brightness;
        //     led[il].b=yellow[2]*brightness;
        //  }
        //  if( il >= _START_CYAN && il < _MAX_LED ) { 
        //     led[il].r=cyan[0]*brightness;
        //     led[il].g=cyan[1]*brightness;
        //     led[il].b=cyan[2]*brightness;
        //  }
        //}

        ws2812_setleds(led,_MAX_LED);
        //_delay_ms(DELAY);                         // wait for 500ms.


      }
      else if( ipat ==  7 ) { 
              // drive slave select low
          PORTB &= ~(1 << CS);
          _delay_us(10);                         // wait for 500ms.

          uint16_t temp_data0 = 0;
          uint16_t temp_data1 = 0;
          ////// receive 4 bytes
          temp_data1 = SPI_masterReceive();
          temp_data1 = temp_data1 << 8;
          temp_data1 = temp_data1 | SPI_masterReceive();

          temp_data0 = SPI_masterReceive();
          temp_data0 = temp_data0 << 8;
          temp_data0 = temp_data0 | SPI_masterReceive();

          // return slave select to high
          PORTB |= (1 << CS);
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=0;
              led[il].g=0;
              led[il].b=0;
          }

          for(int ibit = 0; ibit < 16; ++ibit )
          {
              if( temp_data0 & ( 1 << ibit ) ) {
                  led[ibit].r=violet[0]*4;
                  led[ibit].g=violet[1]*4;
                  led[ibit].b=violet[2]*4;
              }
          }
          for(int ibit = 0; ibit < 16; ++ibit )
          {
              if( temp_data1 & ( 1 << ibit ) ) {
                  led[ibit+16].r=violet[0]*4;
                  led[ibit+16].g=violet[1]*4;
                  led[ibit+16].b=violet[2]*4;
              }
          }
        ws2812_setleds(led,_MAX_LED);
        //_delay_ms(DELAY);                         // wait for 500ms.

      }

      istep++;
  }

}
