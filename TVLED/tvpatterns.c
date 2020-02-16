/*
* Light_WS2812 library example - RGB_blinky
*
* cycles one LED through red, green, blue
*
* This example is configured for a ATtiny85 with PLL clock fuse set and
* the WS2812 string connected to PB4.
*/

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "light_ws2812.h"

#define _MAX_LED 82
#define _MAX_DELAY 1024

volatile int on_led = 1;
struct cRGB led[_MAX_LED];

int DELAY = 64;
int BUTTON_VETO = 0;

uint8_t TCCR1B_SEL = (1 << CS11 ) | (1 << CS10 );
  


ISR( TIMER1_OVF_vect ) {
    
    BUTTON_VETO = 0;
    TCCR1B &= ~TCCR1B_SEL;

}

ISR(INT0_vect)
{
    if (BUTTON_VETO == 0 ) { 
        on_led+=1;
        if( on_led > _MAX_LED ) { 
            on_led = 1;
            for( int il = 0 ; il < _MAX_LED; il++ ) {
                led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
            }
            ws2812_setleds(led,_MAX_LED);
        }
        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

ISR(INT1_vect)
{
    if (BUTTON_VETO == 0 ) { 
        if( DELAY == 1 ) {
            DELAY = _MAX_DELAY;
        }
        DELAY = DELAY/2;
        
        BUTTON_VETO = 1;
        TCCR1B |= TCCR1B_SEL;
    }

}

int main(void)
{
  //int on_val = 16;

  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1<<PD2 ) | (1 << PD3 );   // enable PORTD.2, PORTD.3 pin pull up resistor
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge
  sei();
  _delay_us(100);
  while(1)
  {
      // ************************
      
      
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=32;led[il].g=0;led[il].b=0;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=32;led[il].b=0;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=32;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.
      //
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=16;led[il].g=0;led[il].b=0;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=16;led[il].b=0;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=16;    // Write red to array
      //      ws2812_setleds(led,il);
      //      _delay_ms(DELAY);                         // wait for 500ms.
      //}
      //for( int il = 0 ; il < max_led; il++ ) {
      //      led[il].r=0;led[il].g=0;led[il].b=0;    // Write red to array
      //}
      //ws2812_setleds(led,max_led);
      //_delay_ms(DELAY);                         // wait for 500ms.



      // ************************
      // Color loop test
      for( int ir = 0; ir < 32; ir+=2 ) { 
          for( int ig = 0; ig < 32; ig+=2 ) { 
              for( int ib = 0; ib < 32; ib+=2 ) { 

                  for( int il = 0 ; il < on_led ; il++ ) {
                      led[il].r=ir;led[il].g=ig;led[il].b=ib;    // Write red to array
                  }
                  ws2812_setleds(led,_MAX_LED);
                   _delay_ms(DELAY);                         // wait for 500ms.
              }
          }
      }

  }
}
