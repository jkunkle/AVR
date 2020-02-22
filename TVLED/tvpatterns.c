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
#define _MAX_DELAY 2048
#define _MIN_DELAY 32
#define _N_STEPS 82
#define _N_PAT 2

volatile int on_led = 1;
struct cRGB led[_MAX_LED];

volatile int DELAY = _MAX_DELAY;
//int pattern[_N_PAT][_N_STEPS][_MAX_LED]; 
int pattern[_N_STEPS][_MAX_LED]; 

int pattern_steps[_N_PAT] = {3, 82};

int BUTTON_VETO = 0;

int violet[4] = {8, 0, 1, 1};
int cyan[4] = {0, 3, 4, 1};
int yellow[4] = {8, 3, 0, 1};
int beige[4] = {8, 3, 1, 1};

volatile int ipat = 0;


uint8_t TCCR1B_SEL = (1 << CS11 ) | (1 << CS10 );
  


ISR( TIMER1_OVF_vect ) {
    
    BUTTON_VETO = 0;
    TCCR1B &= ~TCCR1B_SEL;

}

ISR(INT0_vect)
{
    if (BUTTON_VETO == 0 ) { 

        ipat += 1;
        if( ipat >= _N_PAT ) { 
            ipat = 0;
        }

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
  pattern[0][0]=8; 
  pattern[0][1]=8; 
  pattern[0][2]=8; 
  pattern[0][3]=8; 
  pattern[0][4]=8; 
  pattern[0][5]=8; 
  pattern[0][6]=8; 
  pattern[0][7]=8; 
  pattern[0][8]=8; 
  pattern[0][9]=8; 
  pattern[0][10]=8; 
  pattern[0][11]=8; 
  pattern[0][12]=8; 
  pattern[0][13]=8; 
  pattern[0][14]=8; 
  pattern[0][15]=8; 
  pattern[0][16]=8; 
  pattern[0][17]=8; 
  pattern[0][18]=8; 
  pattern[0][19]=8; 
  pattern[0][20]=8; 
  pattern[0][21]=8; 
  pattern[0][22]=8; 
  pattern[0][23]=8; 
  pattern[0][24]=8; 
  pattern[0][25]=8; 
  pattern[0][26]=8; 
  pattern[0][27]=8; 
  pattern[0][28]=8; 
  pattern[0][29]=8; 
  pattern[0][30]=8; 
  pattern[0][31]=8; 
  pattern[0][32]=8; 
  pattern[0][33]=8; 
  pattern[0][34]=8; 
  pattern[0][35]=8; 
  pattern[0][36]=8; 
  pattern[0][37]=8; 
  pattern[0][38]=0; 
  pattern[0][39]=0;
  pattern[0][40]=0; 
  pattern[0][41]=0; 
  pattern[0][42]=0; 
  pattern[0][43]=0; 
  pattern[0][44]=0; 
  pattern[0][45]=0; 
  pattern[0][46]=0; 
  pattern[0][47]=0;
  pattern[0][48]=0; 
  pattern[0][49]=0; 
  pattern[0][50]=0; 
  pattern[0][51]=0; 
  pattern[0][52]=0; 
  pattern[0][53]=0; 
  pattern[0][54]=0; 
  pattern[0][55]=0;
  pattern[0][56]=0; 
  pattern[0][57]=0; 
  pattern[0][58]=0; 
  pattern[0][59]=0; 
  pattern[0][60]=0; 
  pattern[0][61]=0; 
  pattern[0][62]=0; 
  pattern[0][63]=0;
  pattern[0][64]=0; 
  pattern[0][65]=0; 
  pattern[0][66]=0; 
  pattern[0][67]=0; 
  pattern[0][68]=0; 
  pattern[0][69]=0; 
  pattern[0][70]=0; 
  pattern[0][71]=0;
  pattern[0][72]=0; 
  pattern[0][73]=0; 
  pattern[0][74]=0; 
  pattern[0][75]=0; 
  pattern[0][76]=0; 
  pattern[0][77]=0; 
  pattern[0][78]=0; 
  pattern[0][79]=0;
  pattern[0][80]=0; 
  pattern[0][81]=0;
  
  pattern[1][0]=0; 
  pattern[1][1]=0; 
  pattern[1][2]=0; 
  pattern[1][3]=0; 
  pattern[1][4]=0; 
  pattern[1][5]=0; 
  pattern[1][6]=0; 
  pattern[1][7]=0; 
  pattern[1][8]=0; 
  pattern[1][9]=0; 
  pattern[1][10]=0; 
  pattern[1][11]=0; 
  pattern[1][12]=0; 
  pattern[1][13]=0; 
  pattern[1][14]=0; 
  pattern[1][15]=0; 
  pattern[1][16]=0; 
  pattern[1][17]=0; 
  pattern[1][18]=0; 
  pattern[1][19]=0; 
  pattern[1][20]=0; 
  pattern[1][21]=0; 
  pattern[1][22]=0; 
  pattern[1][23]=0; 
  pattern[1][24]=0; 
  pattern[1][25]=0; 
  pattern[1][26]=0; 
  pattern[1][27]=0; 
  pattern[1][28]=0; 
  pattern[1][29]=0; 
  pattern[1][30]=0; 
  pattern[1][31]=0; 
  pattern[1][32]=0; 
  pattern[1][33]=0; 
  pattern[1][34]=0; 
  pattern[1][35]=0; 
  pattern[1][36]=0; 
  pattern[1][37]=0; 
  pattern[1][38]=8; 
  pattern[1][39]=8;
  pattern[1][40]=8; 
  pattern[1][41]=8; 
  pattern[1][42]=8; 
  pattern[1][43]=8; 
  pattern[1][44]=8; 
  pattern[1][45]=8; 
  pattern[1][46]=8; 
  pattern[1][47]=8;
  pattern[1][48]=8; 
  pattern[1][49]=8; 
  pattern[1][50]=8; 
  pattern[1][51]=8; 
  pattern[1][52]=8; 
  pattern[1][53]=8; 
  pattern[1][54]=8; 
  pattern[1][55]=8;
  pattern[1][56]=8; 
  pattern[1][57]=8; 
  pattern[1][58]=8; 
  pattern[1][59]=8; 
  pattern[1][60]=8; 
  pattern[1][61]=8; 
  pattern[1][62]=8; 
  pattern[1][63]=8;
  pattern[1][64]=8; 
  pattern[1][65]=8; 
  pattern[1][66]=0; 
  pattern[1][67]=0; 
  pattern[1][68]=0; 
  pattern[1][69]=0; 
  pattern[1][70]=0; 
  pattern[1][71]=0;
  pattern[1][72]=0; 
  pattern[1][73]=0; 
  pattern[1][74]=0; 
  pattern[1][75]=0; 
  pattern[1][76]=0; 
  pattern[1][77]=0; 
  pattern[1][78]=0; 
  pattern[1][79]=0;
  pattern[1][80]=0; 
  pattern[1][81]=0;
  
  pattern[2][0]=0; 
  pattern[2][1]=0; 
  pattern[2][2]=0; 
  pattern[2][3]=0; 
  pattern[2][4]=0; 
  pattern[2][5]=0; 
  pattern[2][6]=0; 
  pattern[2][7]=0; 
  pattern[2][8]=0; 
  pattern[2][9]=0; 
  pattern[2][10]=0; 
  pattern[2][11]=0; 
  pattern[2][12]=0; 
  pattern[2][13]=0; 
  pattern[2][14]=0; 
  pattern[2][15]=0; 
  pattern[2][16]=0; 
  pattern[2][17]=0; 
  pattern[2][18]=0; 
  pattern[2][19]=0; 
  pattern[2][20]=0; 
  pattern[2][21]=0; 
  pattern[2][22]=0; 
  pattern[2][23]=0; 
  pattern[2][24]=0; 
  pattern[2][25]=0; 
  pattern[2][26]=0; 
  pattern[2][27]=0; 
  pattern[2][28]=0; 
  pattern[2][29]=0; 
  pattern[2][30]=0; 
  pattern[2][31]=0; 
  pattern[2][32]=0; 
  pattern[2][33]=0; 
  pattern[2][34]=0; 
  pattern[2][35]=0; 
  pattern[2][36]=0; 
  pattern[2][37]=0; 
  pattern[2][38]=0; 
  pattern[2][39]=0;
  pattern[2][40]=0; 
  pattern[2][41]=0; 
  pattern[2][42]=0; 
  pattern[2][43]=0; 
  pattern[2][44]=0; 
  pattern[2][45]=0; 
  pattern[2][46]=0; 
  pattern[2][47]=0;
  pattern[2][48]=0; 
  pattern[2][49]=0; 
  pattern[2][50]=0; 
  pattern[2][51]=0; 
  pattern[2][52]=0; 
  pattern[2][53]=0; 
  pattern[2][54]=0; 
  pattern[2][55]=0;
  pattern[2][56]=0; 
  pattern[2][57]=0; 
  pattern[2][58]=0; 
  pattern[2][59]=0; 
  pattern[2][60]=0; 
  pattern[2][61]=0; 
  pattern[2][62]=0; 
  pattern[2][63]=0;
  pattern[2][64]=0; 
  pattern[2][65]=0; 
  pattern[2][66]=8; 
  pattern[2][67]=8; 
  pattern[2][68]=8; 
  pattern[2][69]=8; 
  pattern[2][70]=8; 
  pattern[2][71]=8;
  pattern[2][72]=8; 
  pattern[2][73]=8; 
  pattern[2][74]=8; 
  pattern[2][75]=8; 
  pattern[2][76]=8; 
  pattern[2][77]=8; 
  pattern[2][78]=8; 
  pattern[2][79]=8;
  pattern[2][80]=8; 
  pattern[2][81]=8;

  //for( int i = 0; i < _MAX_LED; ++i ) { 
  //    for( int j = 0; j < _MAX_LED; ++j ) { 
  //        if( i == j ) { 
  //            pattern[1][j][i] = 4;
  //        } else {
  //            pattern[1][j][i] = 0;
  //        }
  //    }
  //}

  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1<<PD2 ) | (1 << PD3 );   // enable PORTD.2, PORTD.3 pin pull up resistor
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge
  sei();
  _delay_us(100);
  // ************************
  // Finding TV purple

  while(1)
  {
      // ************************
      
      for( int ip =0; ip < 3; ip++ ) {
          for( int il = 0 ; il < _MAX_LED; il++ ) {
              led[il].r=beige[0]*pattern[ip][il];
              led[il].g=beige[1]*pattern[ip][il];
              led[il].b=beige[2]*pattern[ip][il];    // Write red to array
          }
          ws2812_setleds(led,_MAX_LED);
          _delay_ms(DELAY);                         // wait for 500ms.
          //if( ip == pattern_steps[ipat] - 1 ) { 
          //    break;
          //}
      }



      
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



      //// ************************
      //// Color loop test
      //for( int ir = 0; ir < 32; ir+=2 ) { 
      //    for( int ig = 0; ig < 32; ig+=2 ) { 
      //        for( int ib = 0; ib < 32; ib+=2 ) { 

      //            for( int il = 0 ; il < on_led ; il++ ) {
      //                led[il].r=ir;led[il].g=ig;led[il].b=ib;    // Write red to array
      //            }
      //            ws2812_setleds(led,_MAX_LED);
      //             _delay_ms(DELAY);                         // wait for 500ms.
      //        }
      //    }
      //}


  }
}
