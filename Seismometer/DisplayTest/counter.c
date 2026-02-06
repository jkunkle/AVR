#define F_CPU 1000000
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SHIFT_RESET PB0
#define SHIFT_COPY PD7
#define SHIFT_ENABLE PD6
#define SHIFT_DATA PD5

uint16_t count;
uint8_t time_comp;
uint16_t time;
uint8_t timer_l;
uint8_t timer_h;
uint16_t diff;
uint16_t deg_per_s;

const uint16_t n_slots = 20;

const uint8_t number_map[10] PROGMEM = {
    0x3f,
    0x06,
    0x5b,
    0x4f,
    0x66,
    0x6d,
    0x7d,
    0x07,
    0x7f,
    0x6f
};
  
void write_to_shift( uint8_t cathode, uint8_t annode )
{
    // disable output
    PORTD = PORTD | (0x1 << SHIFT_ENABLE);
    for(int dd = 3; dd >= 0; --dd)
    {
        if( cathode & ( 0x1 << dd ) )
        {
            PORTD = PORTD | ( 0x1 << SHIFT_DATA );
        }

        PORTD |= (0x1 << SHIFT_COPY );
        PORTD &= ~(0x1 << SHIFT_COPY );
        PORTD = PORTD & ~(0x1 << SHIFT_DATA );
    }


    for(int dd = 7; dd >= 0; --dd)
    {
        if( annode & ( 0x1 << dd ) )
        {
            PORTD = PORTD | ( 0x1 << SHIFT_DATA );
        }

        PORTD |= (0x1 << SHIFT_COPY );
        PORTD &= ~(0x1 << SHIFT_COPY );
        PORTD = PORTD & ~(0x1 << SHIFT_DATA );
    }

    // one more copy is needed
    PORTD |= (0x1 << SHIFT_COPY );
    PORTD &= ~(0x1 << SHIFT_COPY );
    //// enable
    PORTD &= ~(0x1 << SHIFT_ENABLE);
}

uint8_t get_segment_pattern(int value)
{
    if( value < 0 )
    {
        return 0;
    }
    else
    {
        return pgm_read_word(&(number_map[value]));
    }

}
   
void display_digits(int num0, int num1, int num2, int num3, int time)
{

    uint8_t seg0 = get_segment_pattern(num0);
    uint8_t seg1 = get_segment_pattern(num1);
    uint8_t seg2 = get_segment_pattern(num2);
    uint8_t seg3 = get_segment_pattern(num3);
    for( int i = 0 ; i < time; i++ ) {
        write_to_shift( 0x01, seg3);
        _delay_us( 100 );
        write_to_shift( 0x02, seg1);
        _delay_us( 100 );
        write_to_shift( 0x04, seg2);
        _delay_us( 100 );
        write_to_shift( 0x08, seg0);
        _delay_us( 100 );
    }


}

void display_number(uint16_t number, int time)
{

    int thousands = number/1000;
    int hundreds = (number-thousands*1000)/100;
    int tens = (number-thousands*1000-hundreds*100)/10;
    int ones = number-thousands*1000-hundreds*100-tens*10;

    if( thousands == 0 ) {
        thousands = -1;
        if( hundreds == 0) { 
            hundreds = -1;
            if( tens == 0 ) { 
                tens = -1;
            }
        }
    }

    display_digits(thousands, hundreds, tens, ones, time);

}

void reset_shift(void)
{
    PORTB &= ~(0x01 << SHIFT_RESET);
    _delay_us(1);
    PORTB |= (0x01 << SHIFT_RESET);
}

int main(void)
{
  DDRD |= (1 << PD5 ) | (1 << PD6 ) | (1 << PD7) | (0 << PD2);
  DDRB |= ( 1 << PB0 ); 
  EICRA = 1<<ISC01 | 1<<ISC00;
  EIMSK = 1<<INT0;
  PORTD |= 0<<PD2;

  //TCCR0B = ((1 << CS02) | (1 << CS00));
  TCCR1B = ((1 << CS12) | (1 << CS10));
  //TCCR1B = ((1 << CS12));

  reset_shift();

  count = 0;
  time_comp = 0;
  time = 0;
  diff = 0;
  _delay_us(500);
  while(1) {

      display_number(2197, 100);
  }
}

