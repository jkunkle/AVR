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

#define _MAX_LED 82
#define _MAX_DELAY 1024
#define _MIN_DELAY 32
#define _N_STEPS 82
#define _N_PAT 2

struct cRGB led[_MAX_LED];

//volatile int DELAY = _MAX_DELAY;
volatile int DELAY = 64;
//int pattern[_N_PAT][_N_STEPS][_MAX_LED]; 
int pattern[_N_STEPS][_MAX_LED]; 

//int pattern_steps[_N_PAT] = {3, 82};

int BUTTON_VETO = 0;

//int violet[4] = {8, 0, 1, 1};
//int cyan[4] = {0, 3, 4, 1};
//int yellow[4] = {8, 3, 0, 1};
//int beige[4] = {8, 3, 1, 1};

//volatile int ipat = 0;

uint8_t TCCR1B_SEL = (1 << CS11 ) | (1 << CS10 );


ISR( TIMER1_OVF_vect ) {
    
    BUTTON_VETO = 0;
    TCCR1B &= ~TCCR1B_SEL;

}

ISR(INT0_vect)
{
    if (BUTTON_VETO == 0 ) { 

        //ipat += 1;
        //if( ipat >= _N_PAT ) { 
        //    ipat = 0;
        //}

        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

ISR(INT1_vect)
{
    if (BUTTON_VETO == 0 ) { 
        DELAY /= 2;
        if( DELAY <= _MIN_DELAY ) DELAY=_MAX_DELAY;
        
        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

int main(void)
{

  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1<<PD2 ) | (1 << PD3 );   // enable PORTD.2, PORTD.3 pin pull up resistor
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge
  sei();

  _delay_us(100);
  while(1) {

  //    
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=32;led[il].g=0;led[il].b=0;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=32;led[il].b=0;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=32;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.
  //    //
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=16;led[il].g=0;led[il].b=0;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=16;led[il].b=0;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=16;    // Write red to array
  //    //      ws2812_setleds(led,il);
  //    //      _delay_ms(DELAY);                         // wait for 500ms.
  //    //}
  //    //for( int il = 0 ; il < max_led; il++ ) {
  //    //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
  //    //}
  //    //ws2812_setleds(led,max_led);
  //    //_delay_ms(DELAY);                         // wait for 500ms.



  // ************************
  // Color loop test
  for( int ir = 0; ir < 32; ir+=2 ) { 
      for( int ig = 0; ig < 32; ig+=2 ) { 
          for( int ib = 0; ib < 32; ib+=2 ) { 

              for( int il = 0 ; il < _MAX_LED; il++ ) {
                  led[il].r=ir;led[il].g=ig;led[il].b=ib;    // Write red to array
              }
              ws2812_setleds(led,_MAX_LED);
               _delay_ms(DELAY);                         // wait for 500ms.
          }
      }
  }
}


  //}
}
