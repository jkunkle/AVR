//
// LED Driver for TV sign
//
//
// Utilizes Light_WS2812 library for LED communication

// These are the main TV colors
// The colors must be matched to
// RGB values. Since this is light
// and not pigment, the produced
// colors will not perfectly replicate
// the original colors
// VIOLET : #8B3E6E RED=16, BLUE=2-4
// CYAN : #4DBAC3 BLUE=16 GREEN=12-16
// DIRTY YELLOW : #C6C474 RED=16 GREEN=5-6
// EGGSHELL WHITE : #E3E1CD: RED=16, GREEN=6 BLUE=1
//
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "light_ws2812.h"
#include "tvpatterns.h"

// Number of Violet LEDs
#define _N_LED_VIOLET 170
// Number of Beige LEDs
#define _N_LED_BEIGE 83
// Number of Yellow LEDs
#define _N_LED_YELLOW 116
// Number of Cyan LEDs
#define _N_LED_CYAN 170
#define _MAX_LED _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW + _N_LED_CYAN + 1
// Total number of patterns (increase if patterns are added)
#define _N_PAT 6
// Number of steps in race pattern
#define _N_RACE_STEPS 63

// maximum brightness factor
// if it is set too high it 
// will draw too much current
// for the power supply
#define _MAX_BRIGHTNESS 6

// define the start LED of each color
#define _START_VIOLET 0
#define _START_BEIGE _N_LED_VIOLET
#define _START_YELLOW _N_LED_VIOLET + _N_LED_BEIGE
#define _START_CYAN _N_LED_VIOLET + _N_LED_BEIGE + _N_LED_YELLOW

// defines for buttons
#define BUTTON_PATTERN 0
#define BUTTON_SPEED 1
#define BUTTON_BRIGHT 2

FUSES = 
{
    .low = 0xff, 
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};


// Store the colors for all LEDs
struct cRGB led[_MAX_LED];


// Minimum brightness color patterns
// for each displayed color
// order is {R, G, B}
volatile uint8_t violet[3] = {8, 0, 1};
volatile uint8_t cyan[3] = {0, 3, 4};
volatile uint8_t yellow[3] = {8, 3, 0};
volatile uint8_t beige[3] = {8, 3, 1};

// define the delay limits for each pattern
// these should be set so that the pattern
// cannot become too slow or too fast
const uint8_t max_delays[_N_PAT] = {16,64, 64, 64, 64, 32};
const uint8_t min_delays[_N_PAT] = {16,1 , 1 , 1 , 1 , 1};

// default the starting delay
volatile uint8_t DELAY = max_delays[0];

const uint8_t MAX_RACE_WIDTH = 40;
volatile uint8_t race_width = 8;
volatile uint8_t sparkle_count = 8;

// Define the LED configurations for each step
// in the race pattern.  There are 12 entries
// per step, separated to have 3 per color where
// each of the 3 corresponds to a different LED ring
// entries 0-2 = violet
// entries 3-5 = beige 
// entries 6-8 = yellow
// entries 9-11 = cyan
// generally the first in a group corresponds to the
// inner ring, the second to the middle ring
// and the third to the outer ring
const uint16_t steps_race[_N_RACE_STEPS][12] PROGMEM = {
                {0,   92,  120, 172, 210, 237, 255, 308, 347, 539, 513, 512},
                {1,   93,  119, 172, 210, 237, 256, 309, 348, 538, 514, 511},
                {2,   94,  118, 173, 211, 238, 257, 310, 349, 537, 515, 510},
                {3,   95,  117, 173, 211, 238, 258, 311, 350, 536, 516, 509},
                {4,   96,  116, 174, 212, 239, 259, 312, 351, 535, 517, 508},
                {5,   97,  115, 174, 212, 239, 260, 313, 352, 534, 518, 507},
                {6,   98,  114, 175, 213, 240, 261, 314, 353, 533, 519, 506},
                {7,   99,  113, 175, 213, 240, 262, 315, 354, 532, 520, 505},
                {8,   100, 112, 176, 214, 241, 263, 316, 355, 531, 521, 504},
                {9,   101, 111, 177, 215, 241, 264, 317, 356, 530, 522, 503},
                // corner
                {9,   102, 110, 178, 216, 241, 265, 318, 357, 530, 523, 502},
                {9,   102, 109, 179, 217, 241, 266, 319, 357, 530, 523, 501},
                {10,  103, 108, 180, 218, 242, 267, 320, 357, 529, 524, 500},
                {10,  104, 107, 181, 219, 242, 268, 321, 357, 529, 525, 499},
                {10,  105, 106, 182, 220, 243, 269, 322, 357, 529, 525, 498},
                {10,  11,  12,  183, 221, 243, 270, 323, 357, 529, 528, 527},

                {10,  11,  13,  183, 221, 243, 271, 324, 357, 529, 528, 461},
                {10,  52,  14,  183, 221, 243, 272, 325, 357, 529, 462, 460},
                // together again
                {53,  51,  15,  178, 216, 243, 273, 325, 357, 497, 463, 459},
                {54,  50,  16,  178, 216, 243, 274, 325, 357, 496, 464, 458},
                {55,  49,  17,  178, 216, 243, 275, 325, 357, 495, 465, 457},
                {56,  48,  18,  178, 216, 243, 276, 325, 357, 494, 466, 456},
                {57,  47,  19,  178, 216, 243, 277, 325, 357, 493, 467, 455},
                {58,  46,  20,  178, 216, 243, 278, 325, 357, 492, 468, 454},
                {59,  45,  21,  178, 216, 243, 279, 325, 357, 491, 469, 453},
                {60,  44,  22,  178, 216, 243, 280, 325, 357, 490, 470, 452},
                {61,  43,  23,  178, 216, 243, 281, 325, 357, 489, 471, 451},
                {62,  42,  24,  178, 216, 243, 282, 325, 357, 488, 472, 450},
                {63,  41,  25,  178, 216, 243, 283, 326, 357, 487, 473, 449},
                {64,  40,  26,  178, 216, 243, 284, 327, 357, 486, 474, 448},
                {65,  39,  27,  178, 216, 243, 285, 328, 357, 485, 475, 447},
                {66,  38,  28,  178, 216, 243, 286, 329, 357, 484, 476, 446},
                {67,  37,  29,  178, 216, 243, 287, 330, 357, 483, 477, 445},
                {68,  36,  30,  178, 216, 243, 288, 331, 357, 482, 478, 444},
                {69,  35,  31,  178, 216, 243, 289, 332, 357, 481, 479, 443},
                // corner
                {69,  34,  32,  178, 216, 243, 290, 333, 358, 481, 480, 442},
                {69,  34,  33,  178, 216, 243, 291, 334, 359, 481, 480, 441},
                {70,  144, 145, 178, 216, 243, 292, 335, 360, 440, 394, 393},
                {70,  143, 146, 178, 216, 243, 293, 336, 361, 440, 394, 392},
                {70,  143, 147, 178, 216, 243, 294, 337, 362, 440, 395, 391},
                {70,  142, 148, 178, 216, 243, 295, 338, 363, 439, 396, 390},
                // together again
                {71,  141, 149, 178, 216, 243, 296, 339, 364, 438, 397, 389},
                {72,  140, 150, 178, 216, 243, 297, 340, 365, 437, 398, 388},
                {73,  139, 151, 178, 216, 243, 298, 341, 366, 436, 399, 387},
                {74,  138, 152, 178, 216, 243, 299, 342, 366, 435, 400, 386},
                {75,  137, 153, 178, 216, 243, 300, 342, 366, 434, 401, 385},
                {76,  136, 154, 178, 216, 243, 301, 343, 367, 433, 402, 384},
                {77,  135, 155, 178, 216, 243, 302, 344, 367, 432, 403, 383},
                {78,  134, 156, 178, 216, 243, 303, 344, 367, 431, 404, 382},
                {79,  133, 157, 178, 216, 243, 304, 344, 367, 430, 405, 381},
                {80,  132, 158, 178, 216, 243, 305, 345, 367, 429, 406, 380},
                {81,  131, 159, 178, 216, 243, 306, 346, 368, 428, 407, 379},
                {82,  130, 160, 178, 216, 243, 253, 307, 347, 427, 408, 378},
                {83,  129, 161, 178, 216, 243, 254, 307, 347, 426, 409, 377},
                {84,  128, 162, 178, 216, 243, 255, 308, 347, 425, 410, 376},
                {85,  127, 163, 178, 216, 243, 255, 307, 347, 424, 411, 375},
                {86,  126, 164, 178, 216, 243, 255, 307, 347, 423, 412, 374},
                {87,  125, 165, 178, 216, 243, 255, 307, 347, 422, 413, 373},
                {88,  124, 166, 178, 216, 243, 255, 307, 347, 421, 414, 372},
                // corner
                {89,  123, 167, 178, 216, 243, 255, 307, 347, 420, 415, 371},
                {89,  122, 168, 178, 216, 243, 255, 307, 347, 420, 416, 370},
                {89,  122, 169, 178, 216, 243, 255, 307, 347, 420, 416, 369},
                {89,  90,  91,  178, 216, 243, 255, 307, 347, 420, 419, 418},
                            };

// define a mapping of colors
// to sides for the switch pattern
const uint8_t color_patterns[12][4] PROGMEM = {
                {0, 1, 2, 3},
                {1, 2, 3, 0},
                {2, 3, 0, 1},
                {3, 0, 1, 2},
                {0, 2, 1, 3},
                {1, 0, 3, 2},
                {2, 0, 1, 3},
                {3, 1, 0, 2},
                {0, 3, 1, 2},
                {1, 0, 2, 3},
                {2, 3, 0, 1},
                {3, 2, 1, 0},
};

// store the ranges of each set of
// colors for convenience in the
// switch pattern
const uint16_t color_ranges[4][2] PROGMEM = {
    {_START_VIOLET, _START_BEIGE},
    {_START_BEIGE, _START_YELLOW},
    {_START_YELLOW, _START_CYAN},
    {_START_CYAN, _MAX_LED},
};


//store the current pattern
volatile int ipat = 0; 
//store the current step
volatile uint16_t istep = 0;

//a bool for ending updates
//currently only used for the startup pattern
volatile int stop_updates = 0;

// variables for startup pattern
volatile int idirection = 1;
volatile int isub = 0;

// brightness, default to maximum
volatile uint8_t brightness = _MAX_BRIGHTNESS;

// storage for active buttons
volatile uint8_t active_buttons = 0;

// define interrupts
uint8_t TCCR1B_SEL = (1 << CS11 ) | (1 << CS10 );

// define interrupt for receiving
// data from bluetooth module
ISR(USART_RX_vect)
{
    uint8_t res1 = USART_Receive();
    uint8_t res2 = USART_Receive();

    if( res1 == 0x4a && res2 == 0x01 ) {
        // Move to the next pattern
        update_pattern();
    }
    if( res1 == 0x4a && res2 == 0x02 ) {
        // Increase speed
        // If at max speed, return
        // to slowest
        update_speed();
    }
    if( res1 == 0x4a && res2 == 0x03 ) {
        // Decrease brightness
        // If at min brightness, return
        // to maximum
        update_brightness();
    }
    if( res1 == 0xa4){
        // map one color (res2) 
        // into new RGB values
        // Each of 1 byte
        //
        // Transmit a byte to 
        // request additional
        // data
        USART_Transmit(1);
        uint8_t col1 = USART_Receive();
        uint8_t col2 = USART_Receive();
        uint8_t col3 = USART_Receive();
        change_color(res2, col1, col2, col3);
    }
    if( res1 == 0xa5){
        race_width = res2;
        if (race_width > MAX_RACE_WIDTH) {
            race_width = MAX_RACE_WIDTH;
        }
    }
    if( res1 == 0xa6){
        sparkle_count = res2;
    }
        
}

// define the interrupts for button
// pushes.  
// when a button is pushed, the 
// active_buttons variable is updated
// and a timer is started which will
// catch any additional button pushes
// within the window.  This prevents
// multiple activations from button bounce
// and should be smoother
ISR( TIMER1_OVF_vect ) {
    
    TCCR1B &= ~TCCR1B_SEL;

    if( active_buttons & (1 << BUTTON_PATTERN) ){
        update_pattern();
    }
    else if( active_buttons & ( 1 << BUTTON_SPEED ) ) {
        update_speed();
    }
    else if( active_buttons & (1 << BUTTON_BRIGHT ) ) {
        update_brightness();
    }
    active_buttons = 0;

}

ISR(INT0_vect)
{
    active_buttons |= (1 << BUTTON_PATTERN);
    TCCR1B |= TCCR1B_SEL;
}

ISR(INT1_vect)
{
    active_buttons |= (1 << BUTTON_SPEED);
    TCCR1B |= TCCR1B_SEL;
}

ISR(PCINT2_vect){
    
    active_buttons |= (1 << BUTTON_BRIGHT);
    TCCR1B |= TCCR1B_SEL;
}

// Update to the next pattern
// or return to the first pattern if
// at the end
// also set all LEDs to off in prep
// for next pattern
void update_pattern()
{
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
}

// update the speed of the pattern
// store the step at which the
// current pattern would be and
// fix any jumps in the pattern
// that the delay change would cause
void update_speed()
{
    int prev_step = istep/DELAY;
    DELAY /= 2;
    if( istep/DELAY > (prev_step + 1) ) {
        istep = ( prev_step + 1)*DELAY;
    }
    if( DELAY <= min_delays[ipat]) DELAY=max_delays[ipat];
}

// decrease brightness
// if at minium go to maximum
void update_brightness()
{
    brightness = brightness - 1;
    if( brightness <= 0 )
    {
        brightness = _MAX_BRIGHTNESS;
    }
}

int main(void)
{
  TIMSK1 = ( 1 << TOIE1 );
  PORTD |= ( 1 << PD2 ) | (1 << PD3 ) | (1 << PD4 ); // enable PORTD.2, PORTD.3, PORTD.4 pin pull up resistor
  DDRD |= ( 1 << PD5 ) | ( 1 << PD6 ) | ( 1 << PD7 );
  DDRB |= ( 1 << PB0 );
  EIMSK |= (1<<INT0) | ( 1 << INT1 );  // enable external interrupt 0
  EICRA |= (1<<ISC01) | (1 << ISC11 ); // interrupt on falling edge
  // enable interrupt on PCINT20
  PCICR |= (1 << PCIE2);
  PCMSK2 = 0;
  PCMSK2 |= (1 << PCINT20);

  sei();

  // initialize bluetooth interface
  USART_Init(207);

  _delay_ms(100);
  while(1) {
      
    if( ipat == 0 ) { 
       run_turnon();
    }
    if( ipat == 1 ) { 
        run_wave();
    }
    if( ipat == 2 ) { 
        run_switch();
    }
    if( ipat == 3 ) { 
        run_breathe();
    }
    if( ipat == 4 ) { 
        run_race(race_width);
    }
    else if( ipat == 5 ) { 
        run_sparkle();
    }

    istep++;
  }

}


// Turnon pattern
//
// make a gradual increase
// to the maximal brightness
// and then stay there
void run_turnon(){
        
    if( istep > 13 ) { 
        stop_updates = 1;
    }
    if( stop_updates == 1 ) { 
        return;
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
    _delay_ms(DELAY); 

}

// Wave pattern
//
// Illuminate all LEDs on one ring 
// on each side.  At each step move
// one ring out.  Return to first ring 
// from outside ring
void run_wave() {

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

}

// Switch pattern
// Light all LEDs
// swap colors between sides
void run_switch(){

    uint16_t patStep = istep/DELAY;
    if( patStep >= 12 ) { 
        istep = 0;
    }

    uint8_t pat0  = pgm_read_byte(&(color_patterns[patStep][0]));
    uint8_t pat1  = pgm_read_byte(&(color_patterns[patStep][1]));
    uint8_t pat2  = pgm_read_byte(&(color_patterns[patStep][2]));
    uint8_t pat3  = pgm_read_byte(&(color_patterns[patStep][3]));

    uint16_t startv = pgm_read_word(&(color_ranges[pat0][0]));
    uint16_t endv = pgm_read_word(&(color_ranges[pat0][1]));
    uint16_t startb = pgm_read_word(&(color_ranges[pat1][0]));
    uint16_t endb = pgm_read_word(&(color_ranges[pat1][1]));
    uint16_t starty = pgm_read_word(&(color_ranges[pat2][0]));
    uint16_t endy = pgm_read_word(&(color_ranges[pat2][1]));
    uint16_t startc = pgm_read_word(&(color_ranges[pat3][0]));
    uint16_t endc = pgm_read_word(&(color_ranges[pat3][1]));

    for( int il = startv; il < endv; il++ ) {
        led[il].r=violet[0]*brightness;
        led[il].g=violet[1]*brightness;
        led[il].b=violet[2]*brightness;
    }
    for( int il = startb; il < endb; il++ ) {
        led[il].r=beige[0]*brightness;
        led[il].g=beige[1]*brightness;
        led[il].b=beige[2]*brightness;
    }
    for( int il = starty; il < endy; il++ ) {
        led[il].r=yellow[0]*brightness;
        led[il].g=yellow[1]*brightness;
        led[il].b=yellow[2]*brightness;
    }
    for( int il = startc; il < endc; il++ ) {
        led[il].r=cyan[0]*brightness;
        led[il].g=cyan[1]*brightness;
        led[il].b=cyan[2]*brightness;
    }

    ws2812_setleds(led,_MAX_LED);
}

// Breathe pattern
// Illuminate all LEDs
// gradually increase brightness
// with two speeds, then
// reverse 
void run_breathe(){

    
    // when at step 20
    // switch directions 
    if( istep >= 20 ) { 
        istep = 0;
        if( idirection == 0 ) {
            idirection = 1 ;
        }
        else {
            idirection = 0 ;
        }
    }

    // fast steps
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
    //slow steps
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
    _delay_ms(DELAY);

}
// Racetrack pattern
//
// Illuminate a section of LEDs
// that travels around the rings
// synchronously

void run_race(uint8_t thickness){

    for( int il = 0 ; il < _MAX_LED; il++ ) {
        led[il].r=0;
        led[il].g=0;
        led[il].b=0;
    }

    int raceStep = istep/DELAY;
    if( raceStep >= _N_RACE_STEPS ) { 
        istep = 0;
        ws2812_setleds(led,_MAX_LED);
    }

    for(int ient=0;  ient < thickness; ient++){

        int this_loc = raceStep + ient;
        if(this_loc > _N_RACE_STEPS){
            this_loc = this_loc - _N_RACE_STEPS;
        }

        int loc_cyan1 = this_loc;
        if( ient == 0 ){
            loc_cyan1 = loc_cyan1 + 1;
        }
        if( ient == thickness-1 ){
            loc_cyan1 = loc_cyan1 - 1;
        }

        uint16_t violet0 = pgm_read_word(&(steps_race[this_loc][0]));
        uint16_t violet1 = pgm_read_word(&(steps_race[this_loc][1]));
        uint16_t violet2 = pgm_read_word(&(steps_race[this_loc][2]));

        uint16_t beige0 = pgm_read_word(&(steps_race[this_loc][3]));
        uint16_t beige1 = pgm_read_word(&(steps_race[this_loc][4]));
        uint16_t beige2 = pgm_read_word(&(steps_race[this_loc][5]));

        uint16_t yellow0 = pgm_read_word(&(steps_race[this_loc][6]));
        uint16_t yellow1 = pgm_read_word(&(steps_race[this_loc][7]));
        uint16_t yellow2 = pgm_read_word(&(steps_race[this_loc][8]));

        uint16_t cyan0 = pgm_read_word(&(steps_race[this_loc][9]));
        uint16_t cyan1 = pgm_read_word(&(steps_race[loc_cyan1][10]));
        //uint16_t cyan1 = pgm_read_word(&(steps_race[this_loc][10]));
        uint16_t cyan2 = pgm_read_word(&(steps_race[this_loc][11]));

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
    }

    ws2812_setleds(led,_MAX_LED);
}
// sparkle pattern
// Randomly select LEDs
// to activate
void run_sparkle(){

    if((istep % DELAY) == 0){
        for( int il = 0 ; il < _MAX_LED; il++ ) {
            led[il].r=0;
            led[il].g=0;
            led[il].b=0;
        }

        uint8_t new_seed = fill_random( led, brightness, (uint8_t)istep );
        for(int i = 0; i < sparkle_count; ++i) {
           new_seed = fill_random( led, brightness, new_seed );
        }

    }
    ws2812_setleds(led,_MAX_LED);
}

// Select random-looking value by 
// applying an xor and bit shift
uint8_t random( uint8_t seed ) { 

    seed ^= seed << 4;
    seed ^= seed >> 7;
    seed ^= seed << 1;

    return seed;

}

// Set colors based on random values
uint8_t fill_random(struct cRGB *led, int brightness, uint8_t seed )
{

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

// Initialize USART for bluetooth
void USART_Init( uint16_t ubrr)
{
    UCSR0A |= (1 << U2X0);
    /*Set baud rate */
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;
    //Enable receiver and transmitter, recieve interrupt */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (3<<UCSZ00);
}

// Receive a byte from bluetooth
uint8_t USART_Receive( void )
{
    // wait for data to be recieved
    while ( !(UCSR0A & (1<<RXC0)) );
    return UDR0;
}

// send a btye by bluetooth
void USART_Transmit( unsigned char data )
{
    //Wait for empty transmit buffer
    while ( !( UCSR0A & (1<<UDRE0)) );

    UDR0 = data;
}


// update color values
void change_color(uint8_t index, uint8_t col1, uint8_t col2, uint8_t col3){

    if(index == 1){
        violet[0] = col1;
        violet[1] = col2;
        violet[2] = col3;
    }
    if(index == 2){
        cyan[0] = col1;
        cyan[1] = col2;
        cyan[2] = col3;
    }
    if(index == 3){
        yellow[0] = col1;
        yellow[1] = col2;
        yellow[2] = col3;
    }
    if(index == 4){
        beige[0] = col1;
        beige[1] = col2;
        beige[2] = col3;
    }
}
