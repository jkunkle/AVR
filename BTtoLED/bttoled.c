#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define SHIFT_DATA PINB0
#define SHIFT_ENABLE PINB1
#define SHIFT_RESET PIND7
#define SHIFT_COPY PIND6

#define LCD_ENABLE PIND5
#define LCD_RS PINB6
#define LCD_RW PINB7

uint8_t patterns[16];

void write_shift_byte( uint8_t data )
{
    // disable output
    PORTB = PORTB | (0x1 << SHIFT_ENABLE);
    for( int dd = 0; dd < 8; dd++ )
    {
        if( data & ( 0x1 << dd ) )
        {
            PORTB = PORTB | ( 0x1 << SHIFT_DATA );
        }

        PORTD |= (0x1 << SHIFT_COPY );
        PORTD &= ~(0x1 << SHIFT_COPY );
        PORTB = PORTB & ~(0x1 << SHIFT_DATA );
    }

    // one more copy is needed
    PORTD |= (0x1 << SHIFT_COPY );
    PORTD &= ~(0x1 << SHIFT_COPY );
}

void write_to_lcd( void )
{

    PORTD |= (0x1 << LCD_ENABLE );
    PORTB = PORTB & ~(0x1 << SHIFT_ENABLE );
    PORTD &= ~( 0x1 << LCD_ENABLE );

}

void lcd_clear(void)
{
    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    write_shift_byte( 0x01 );
    write_to_lcd();
    _delay_us( 2000 );

}
void lcd_home(void)
{
    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    write_shift_byte( 0x02 );
    write_to_lcd();
    _delay_us( 2000 );
}

void display_text( uint8_t * data, uint8_t nEntries)
{
    lcd_clear();
    lcd_home();
    for( int i = 0; i < 8; ++i ) {
        if( i >= nEntries ) return;

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( data[i] );
        write_to_lcd();
        _delay_us( 50 );
    }

    if( nEntries <= 8 ) return;

    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );

    write_shift_byte( 0xc0 );
    write_to_lcd();
    _delay_us( 50 );

    for( int i = 8; i < 16; ++i ) {
        if( i >= nEntries ) return;

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( data[i] );
        write_to_lcd();
        _delay_us( 50 );
    }

}


void init_led( void )
{
    _delay_ms(40);

    // set function
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    PORTB = PORTB & ~( 0x1 << LCD_RS );
    //write_shift_byte( 0x34 );
    write_shift_byte( 0x3c );

    write_to_lcd();

    _delay_us( 40 );

    // display, cursor, blinking, on
    write_shift_byte( 0x0f );

    write_to_lcd();

    _delay_us( 40 );

    // display clear
    write_shift_byte( 0x01 );

    write_to_lcd();

    _delay_us( 40 );

    // entry mode set
    write_shift_byte( 0x07 );
    //write_shift_byte( 0x04 );
    //write_shift_byte( 0x05 );

    write_to_lcd();

    _delay_us( 1600 );

    //write_shift_byte( 0x1f );

    //write_to_lcd();

    //_delay_us( 40 );

}

void USART_Init( uint16_t ubrr)
{
    /*Set baud rate */
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;
    //Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Set frame format: 8data, 2stop bit */
    //UCSR0C = (1<<USBS0)|(3<<UCSZ00);
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (3<<UCSZ00);
}
uint8_t USART_Receive( void )
{
    //Wait for data to be received
    while ( !(UCSR0A & (1<<RXC0)) );

    return UDR0;
}

void USART_Transmit( unsigned char data )
{
    //Wait for empty transmit buffer
    while ( !( UCSR0A & (1<<UDRE0)) );

    UDR0 = data;
}

int main( void ) {

    DDRB |= 0xc3;
    DDRD |= 0xe0;

    USART_Init(51);

    PORTD &= ~(0x01 << SHIFT_RESET);
    _delay_us(1);
    PORTD |= (0x01 << SHIFT_RESET);

    init_led();

    while(1) {

        uint8_t nchar = USART_Receive();
        for( int i = 0; i < nchar; ++i)
        {

            uint8_t pat = USART_Receive();
            //PORTB = PORTB | ( 0x1 << LCD_RS );
            //PORTB = PORTB & ~( 0x1 << LCD_RW );
            //write_shift_byte( pat );
            //write_to_lcd();
            //_delay_ms( 500 );
            patterns[i] = pat;

        }

        display_text( patterns, nchar );
    }


}
