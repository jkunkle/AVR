#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "i2c_master.h"

#define CLOCK_CE PINB7
#define CLOCK_IO PINC4
#define CLOCK_SCLK PINC5

#define SHIFT_DATA PINB0
#define SHIFT_ENABLE PINB1
#define SHIFT_RESET PIND7
#define SHIFT_COPY PIND6

#define LCD_ENABLE PIND5
#define LCD_RS PINB6
#define LCD_RW PINB7

#define SPI_DDR DDRB
#define CS      PB2
#define MOSI    PB3
#define MISO    PB4
#define SCK     PB5

uint8_t test_vals[256];

//AHT10
uint8_t aht10_address_write = 0x70;
uint8_t aht10_address_read = 0x71;
uint8_t aht10_address_measure = 0xac;
uint8_t aht10_address_init = 0xe1;

const uint8_t number_map[10] PROGMEM = {
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39
};

void SPI_init(void)
{
    // set CS, MOSI and SCK to output
    SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // enable SPI, set as master, and clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);

    PORTB |= (1 << CS);
}

uint8_t SPI_masterReceive(void)
{
    // transmit dummy byte
    SPDR = 0xFF;

    // Wait for reception complete
    while(!(SPSR & (1 << SPIF)));

    // return Data Register
    return SPDR;
}
void SPI_masterTransmit(uint8_t data)
{
    // load data into register
    SPDR = data;

    // Wait for transmission complete
    while(!(SPSR & (1 << SPIF)));
}

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

void display_number_split( int hundreds, int tens, int ones, int tenths, int hundredths )
{

    if( hundreds >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[hundreds]));
        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }

    if( tens >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[tens]));
        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }
    if( ones >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[ones]));
        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }

    if( tenths >= 0 && hundredths >= 0 ) 
    {

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( 0x2e );
        write_to_lcd();
        _delay_us( 50 );

        uint8_t pat = pgm_read_word(&(number_map[tenths]));
        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );

        pat = pgm_read_word(&(number_map[hundredths]));
        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }

}

void display_2bytes( uint16_t val )
{
    lcd_clear();
    lcd_home();
    for( int i = 15; i >= 8; --i )
    {
        uint8_t pat;
        if( val & ( 0x01 << i ) )
        {
            pat = pgm_read_word(&(number_map[1]));
        }
        else
        {
            pat = pgm_read_word(&(number_map[0]));
        }

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }

    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );

    write_shift_byte( 0xc0 );
    write_to_lcd();
    _delay_us( 50 );

    for( int i = 7; i >= 0; --i )
    {
        uint8_t pat;
        if( val & ( 0x01 << i ) )
        {
            pat = pgm_read_word(&(number_map[1]));
        }
        else
        {
            pat = pgm_read_word(&(number_map[0]));
        }

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }
}

void display_bytes( uint8_t left, uint8_t right )
{
    lcd_clear();
    lcd_home();
    for( int i = 7; i >= 0; --i )
    {
        uint8_t pat;
        if( left & ( 0x01 << i ) )
        {
            pat = pgm_read_word(&(number_map[1]));
        }
        else
        {
            pat = pgm_read_word(&(number_map[0]));
        }

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }

    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );

    write_shift_byte( 0xc0 );
    write_to_lcd();
    _delay_us( 50 );

    for( int i = 7; i >= 0; --i )
    {
        uint8_t pat;
        if( right & ( 0x01 << i ) )
        {
            pat = pgm_read_word(&(number_map[1]));
        }
        else
        {
            pat = pgm_read_word(&(number_map[0]));
        }

        PORTB = PORTB | ( 0x1 << LCD_RS );
        PORTB = PORTB & ~( 0x1 << LCD_RW );
        write_shift_byte( pat );
        write_to_lcd();
        _delay_us( 50 );
    }
}





void display_number( float number ) {

    int hundreds = number/100;
    int tens = (number-hundreds*100)/10;
    int ones = (number-hundreds*100-tens*10);
    int tenths = (number-hundreds*100-tens*10-ones)*10;
    int hundredths = (number-hundreds*100-tens*10-ones-tenths*0.1)*100;

    if( hundreds == 0) { 
        hundreds = -1;
        if( tens == 0 ) { 
            tens = -1;
            if( ones == 0 ) {
                ones = -1;
            }
        }
    }

    if( tenths == 0 && hundredths == 0 ) {
        tenths = -1;
        hundredths = -1;
    }
    display_number_split( hundreds, tens, ones, tenths, hundredths );

}    

void display_time( uint8_t mins, uint8_t secs)
{

    // clear display 
    lcd_clear();

    // return to home
    lcd_home();
    display_number( mins );

    PORTB = PORTB | ( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    write_shift_byte( 0x3a );
    write_to_lcd();
    _delay_us( 50 );

    display_number( secs );
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



void byte_to_clock( uint8_t byte )
{

    for (uint8_t i = 0; i < 8; ++i)
    {
       if(byte & 0x01) PORTC |= (1<<CLOCK_IO);
       else PORTC &= ~(1<<CLOCK_IO);
       PORTC |= (1<<CLOCK_SCLK);
       PORTC &= ~(1<<CLOCK_SCLK);

       byte >>= 1;
    }
}
    //TWDR = byte;
    //// start transmission of address
    //TWCR = (1<<TWINT) | (1<<TWEN);
    //// wait for end of transmission
    //_delay_us(20);
    //while( !(TWCR & (1<<TWINT)) );
    //for( int i = 0; i < 8; ++i )
    //{
    //    if( byte & (0x1 << i ) )
    //    {
    //        PORTC |= (0x1 << 4 );
    //    }
    //    else 
    //    {
    //        PORTC &= ~(0x1 << 4);
    //    }

    //    //clock high
    //    PORTC |= (0x1 << 5);
    //    _delay_us(10);
    //    //clock low
    //    PORTC &= ~(0x1 << 5);
    //    // data low 
    //    PORTC &= ~(0x1 << 4 );
    //}

//}

uint8_t byte_from_clock( void )
{

    uint8_t i;
    uint8_t R_Byte = 0;
    uint8_t T_Byte = 0;

    DDRC &= ~(1<<CLOCK_IO);
    for (i = 0; i < 8; i++) 
    {
        PORTC |= (1<<CLOCK_SCLK);
        if (PINC & (1<<CLOCK_IO)) T_Byte = (1<<CLOCK_IO);
        else T_Byte = 0x00;
        PORTC &= ~(1<<CLOCK_SCLK);
        T_Byte <<= 1;
        R_Byte >>= 1;
        R_Byte |= T_Byte;
    }
    DDRC |= (1<<CLOCK_IO);
    return (R_Byte & 0x7F);
}

    //TWCR = (1<<TWINT) | (1<<TWEN);
    //// wait for end of transmission
    //_delay_us(20);
    ////while( !(TWCR & (1<<TWINT)) );
    //// return received data from TWDR
    //return TWDR;


    //uint8_t value = 0;
    //for( int i = 0; i < 8; ++i )
    //{
    //    //clock high
    //    PORTC |= (0x1 << 5);
    //    _delay_us(10);

    //    value |= ( PINC4 << i );
    //    _delay_us(10);
    //    //clock low
    //    PORTC &= ~(0x1 << 5);
    //    _delay_us(10);
    //}

    //return value;

//}

void write_to_clock( uint8_t address, uint8_t val )
{

    //// ensure that DDR4 is output
    //DDRC |= (0x1 << 4);

    //raise enable bit
    PORTB |= (0x1 << 7);

    byte_to_clock( address );
    byte_to_clock( val );

    //lower enable bit
    PORTB &= ~(0x1 << 7);

}

uint8_t read_from_clock( uint8_t address  )
{

    // ensure that DDR4 is output
    //DDRC |= (0x1 << 4);

    //raise enable bit
    PORTB |= (0x1 << 7);

    byte_to_clock( address );

    uint8_t value = byte_from_clock();

    //lower enable bit
    PORTB &= ~(0x1 << 7);
    return value;

}

void init_clock(void)
{
    DDRC = (1 << CLOCK_SCLK) | (1 << CLOCK_IO);

    write_to_clock( 0x8e, 0x00 );
    write_to_clock( 0x80, 0x00 );
    write_to_clock( 0x82, 0x00 );
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

void memory_chip_enable(void)
{
    PORTB &= ~(1 << CS);
    _delay_us(10);
}

void memory_chip_disable(void)
{
    PORTB |= (1 << CS);
    _delay_us(10);
}

void memory_enable_write(void)
{

    memory_chip_enable();
    SPI_masterTransmit( 0x06 ); // write enable
    memory_chip_disable();

}

void memory_disable_write(void)
{

    memory_chip_enable();
    SPI_masterTransmit( 0x04 ); // write disable
    memory_chip_disable();

}

void memory_page_write(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t bytes[256] )
{
    memory_enable_write();

    memory_chip_enable();

    SPI_masterTransmit( 0x02 ); //page write
    SPI_masterTransmit( addr_high ); 
    SPI_masterTransmit( addr_low ); 
    SPI_masterTransmit( addr_byte ); 
    for( int i = 0; i < 256; ++i ) 
    {
        SPI_masterTransmit( bytes[i] ); 
    }

    memory_chip_disable();

    _delay_ms(3);
}

uint8_t memory_page_read( uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte )
{
    memory_chip_enable();

    SPI_masterTransmit( 0x03 ); //read
    SPI_masterTransmit( addr_high ); 
    SPI_masterTransmit( addr_low ); 
    SPI_masterTransmit( addr_byte ); 
    uint8_t res = SPI_masterReceive(  ); 

    memory_chip_disable();

    return res;
}
void memory_write_status( uint8_t status1, uint8_t status2 )
{
    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0x01 ); //write status
    SPI_masterTransmit( status1 ); 
    SPI_masterTransmit( status2 ); 
    memory_chip_disable();
    _delay_ms(1000);
}

uint16_t memory_status(void)
{
    memory_chip_enable();

    SPI_masterTransmit( 0x05 ); //status
    uint8_t status1 = SPI_masterReceive( );
    memory_chip_disable();
    memory_chip_enable();
    SPI_masterTransmit( 0x35 ); //status
    uint8_t status2 = SPI_masterReceive( );

    uint16_t combStatus = ((uint16_t)status1 << 8 );
    combStatus |= status2;

    memory_chip_disable();

    return combStatus;
}

void memory_chip_erase(void)
{

    memory_enable_write();

    uint16_t stat = memory_status();
    display_2bytes( stat);
    _delay_ms(2000);

    memory_chip_enable();
    SPI_masterTransmit( 0x60 ); //chip erase
    memory_chip_disable();

    stat = memory_status();
    display_2bytes( stat);
    _delay_ms(2000);
}

void memory_sector_erase(uint8_t block, uint8_t sector)
{

    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0x20 ); //sector erase
    SPI_masterTransmit( block );
    SPI_masterTransmit( sector );
    SPI_masterTransmit( 0x00 );
    memory_chip_disable();
    _delay_ms(400);

}



void memory_reset(void)
{
    memory_chip_enable();
    SPI_masterTransmit( 0x66 ); //enable reset
    memory_chip_disable();

    memory_chip_enable();
    SPI_masterTransmit( 0x99 ); //reset
    memory_chip_disable();
    _delay_ms(5000);
}



int main( void ) {

    //i2c_init();

    DDRB |= 0xc3;
    DDRD |= 0xe0;

    USART_Init(51);

    SPI_init();
    //sei();
    // start with reset off
    //_delay_ms(1000);

    // Reset shift registers
    //PORTB = 0x00; 
    // keep the reset active for some time (can be tuned)
    //_delay_ms(1000);
    PORTD &= ~(0x01 << SHIFT_RESET);
    _delay_us(1);
    PORTD |= (0x01 << SHIFT_RESET);
    init_led();

    // Seems to work correctly 
    // without using init
    //i2c_start( aht10_address_write );
    //i2c_write( aht10_address_init );
    //i2c_stop();
    _delay_ms(2000);

    //init_clock();
    //write_to_clock( 0x80, 0 );

    // *************
    // write to status register

    //memory_reset();
    //memory_write_status( 0x00, 0x00 );

    //for( uint8_t i = 0; i < 128; i++ )
    //{
    //    for( int j = 0; j < 256; j++ )
    //    {
    //        test_vals[j] = i*2;
    //    }

    //    memory_page_write( 0x02, i, 0xff, test_vals );
    //}

    //memory_sector_erase(0x01, 0x01);

    //for( int j = 0; j < 256; j++ )
    //{
    //    test_vals[j] = 0xc5;
    //}
    //memory_page_write( 0x01, 0x01, 0x00, test_vals );


    //for( int i = 0; i < 100; ++i )
    //{
    //    memory_enable_write();
    //    uint16_t stat = memory_status();
    //    display_2bytes( stat);
    //    _delay_ms(1000);
    //    memory_disable_write();
    //    stat = memory_status();
    //    display_2bytes( stat);
    //    _delay_ms(1000);
    //}

    while(1) {

        //*******************************
        // AHT10 cycle
        //i2c_start( aht10_address_write );
        //i2c_write( aht10_address_measure );
        //i2c_stop();
        //_delay_ms(100);

        //i2c_start( aht10_address_read );
        //uint8_t status = i2c_read_ack();
        //uint8_t hHigh = i2c_read_ack();
        //uint8_t hLow = i2c_read_ack();
        //uint8_t htmix = i2c_read_ack();
        //uint8_t tHigh = i2c_read_ack();
        //uint8_t tLow = i2c_read_ack();

        ////uint32_t temperature = ( (uint32_t)(htmix & 0x0f) << 16);
        ////temperature = temperature | ((uint16_t)tHigh << 8 );
        ////temperature = temperature | tLow;

        //uint32_t humidity = ( (uint32_t)(hHigh) << 12);
        //humidity |= ( (uint16_t)(hLow) << 4 );
        //humidity |= ( htmix & (0xf0) );


        ////float tempVal = ((float)(temperature)*0.000190735) - 50.;
        //float humidVal = ((float)(humidity)/10485.76);

        //display_number( humidVal);
        //_delay_ms(1000);
        //*******************************
        //

        //uint8_t secs = read_from_clock( 0x81 );
        //secs = (secs & 0x0F) + ((secs & 0x70) >> 4) * 10;
        //uint8_t mins = read_from_clock( 0x83 );
        //mins = (mins & 0x0F) + ((mins & 0x70) >> 4) * 10;
        //
        //SPI_masterTransmit( 0x4b ); // unique id


        //PORTB &= ~(1 << CS);
        //_delay_us(10);            

        //SPI_masterTransmit( 0x05 );
        //uint8_t status1 = SPI_masterReceive( );
        //uint8_t status2 = SPI_masterReceive( );

        //PORTB |= (1 << CS);
        //_delay_us(10);


        ////display_time( mins, secs );
        //display_bytes( 0xcf, 0xd2);
        //_delay_ms(2000);

        //uint16_t stat = memory_status();
        //display_2bytes( stat);
        //_delay_ms(1000);

        //memory_enable_write();

        //uint16_t stat = memory_status();
        //display_2bytes( stat);
        //_delay_ms(2000);
        //uint8_t val = memory_page_read( 0x01, 0x01, 0x00 );
        //display_bytes( val, 0x00 );
        //_delay_ms(1000);

        uint8_t res1 = USART_Receive();
        uint8_t res2 = USART_Receive();
        if( res1 == 0xd1 && res2==0x55 )
        {
            USART_Transmit(0x3d);
        }
        _delay_ms(2000);
    }


}
