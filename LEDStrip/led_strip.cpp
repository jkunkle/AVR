#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

FUSES = 
{
    .low = 0xff,
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};

int PULSE_COUNT = 0;
int MAX_PULSE = 48;

uint8_t ON_TCCR1B = 0x00;
uint8_t OFF_TCCR1B = 0x00;

uint16_t toggle = 0x0001;

ISR( TIMER1_OVF_vect ) {

    
    OCR1A = OCR1A ^ toggle;
    //PULSE_COUNT += 1;
    //if (PULSE_COUNT >= MAX_PULSE ) {
    //    //TCCR1B &= ~(1 << CS10 );
    //    TCCR1B = OFF_TCCR1B;

    //}
}


int main( void ) {

    DDRB |= (1 << DDB1);

    //ICR1 = 0x000B;
    //ICR1 = 0x0014;
    ICR1 = 0x001C;
    //ICR1 = 0x0FFF;
    //OCR1A = 0x000C;
    OCR1A = 0x0014;
    //OCR1A = 0x03FF;
    TCCR1A |= (1 << COM1A1) | ( 1 << COM1A0 ); // inverted
    //TCCR1A |= (1 << COM1A1) ;
    TCCR1A |= (1 << WGM11 ) ; 
    TCCR1B |= (1 << WGM12 ) | (1 << WGM13 ) ; 
    TIMSK1 = ( 1 << TOIE1 );
    ON_TCCR1B = TCCR1B | (1 << CS10 );
    OFF_TCCR1B = TCCR1B & ~(1 << CS10 );
    sei(); //enable interrupts
    _delay_ms( 1000 ); 
    TCCR1B = ON_TCCR1B;

    //_delay_us( 14 ); // 8 pulses
    //_delay_us( 28 ); // 16 pulses
    _delay_us( 4 ); // 24 pulses
    TCCR1B = OFF_TCCR1B;


    while (1) { 
    }

}
