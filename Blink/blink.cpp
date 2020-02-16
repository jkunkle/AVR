#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>

FUSES = 
{
    ///.low = (FUSE_CKSEL0 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0 ), // E2
    .low = 0xff, 
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};


void delay_ms(int ms)
{
    while (ms--) {
        _delay_us(1000);  // one millisecond
    }
}

int main( void ) {

    DDRB = 0xff;
    _delay_ms( 100 );

    PORTB = 0x00;

    while (1) { 
        PORTB = 0xff;
        _delay_ms( 1000 );
        PORTB = 0x00;
        _delay_ms( 1000 );
    }

}
