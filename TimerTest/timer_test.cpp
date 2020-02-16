#include <avr/io.h>
#include <avr/interrupt.h>

int main(void) { 

    PORTB = 0x0;

    OCR0A = 0xFF;
    //OCR1A = 0x1E84;
    OCR1A = 0x3D08;

    TCCR1B |= (1 << WGM12 );
    TCCR0A |= (1 << WGM01 );

    //TIMSK1 |= (1 << OCIE1A );
    TIMSK1 |= (1 << OCIE1A ) ;
    TIMSK0 |= (1 << OCIE0A ) ;

    //TCCR1B |= (1 << CS12) | (1 << CS10);
    TCCR1B |= (1 << CS11);
    //TCCR1B |= (1 << CS11) | ( 1 << CS00 );
    TCCR0B |= (1 << CS02) ;

    sei();

    while(1) { 

    }

}

ISR( TIMER0_COMPA_vect ) { 

    PORTB = PORTB ^ 0x01;


    //PORTB = ~PORTB;

}

ISR( TIMER1_COMPA_vect ) { 

    PORTB = PORTB ^ 0x02;



}

