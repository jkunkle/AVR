#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "i2c_master.h"
#include "htsensor.h"

#define CLOCK_CE PINC0
#define CLOCK_IO PINC1
#define CLOCK_SCLK PINC2

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

uint8_t  yearStore[18];
uint8_t  monthStore[18];
uint8_t  dateStore[18];
uint8_t  hourStore[18];
uint8_t  minuteStore[18];
uint8_t  secondStore[18];
uint8_t tempStore_3[18];
uint8_t tempStore_2[18];
uint8_t tempStore_1[18];
uint8_t tempStore_0[18];
uint8_t humidStore_3[18];
uint8_t humidStore_2[18];
uint8_t humidStore_1[18];
uint8_t humidStore_0[18];
volatile uint16_t _nRecords = 0;
volatile uint8_t _blockRecords = 0;

//AHT10
uint8_t aht10_address_write = 0x70;
uint8_t aht10_address_read = 0x71;
uint8_t aht10_address_measure = 0xac;
uint8_t aht10_address_init = 0xe1;

volatile uint8_t displayTime = 0;
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

ISR(USART_RX_vect)
{
    uint8_t res1 = USART_Receive();
    uint8_t res2 = USART_Receive();
    // get time
    if(  res1 == 0xd0 && res2==0x55 )
    {
        struct DateTime dt;
        dt.second_bits = USART_Receive();
        dt.minute_bits = USART_Receive();
        dt.hour_bits = USART_Receive();
        dt.date_bits = USART_Receive();
        dt.month_bits = USART_Receive();
        dt.day_bits  = USART_Receive();
        dt.year_bits  = USART_Receive();

        write_datetime_to_clock( dt );
    }
    // reset
    if( res1 == 0xd1 && res2==0x55 )
    {
        // reset time
        struct DateTime dt = read_datetime_from_clock();

        USART_Transmit( dt.second_bits );
        USART_Transmit( dt.minute_bits );
        USART_Transmit( dt.hour_bits );
        USART_Transmit( dt.date_bits );
        USART_Transmit( dt.month_bits );
        USART_Transmit( dt.day_bits );
        USART_Transmit( dt.year_bits );

        // set the number of records back to 0
        // all data will be overwritten
        _nRecords = 0;
        _blockRecords = 0;

    }

    if( res1 == 0xd2 && res2==0x55 )
    {

        uint16_t nPages = _nRecords/18;
        uint8_t nRemain = _nRecords%18;
        // write remaining data to memory
        if( nRemain > 0 )
        {
            write_block_to_memory(nRemain);
            nPages++;
        }
        // transmit number of records
        USART_Transmit( ( _nRecords& 0xff00 ) >> 8 );
        USART_Transmit( _nRecords& 0x00ff );

        // start at page 1
        uint16_t ipage = 1;
        while( ipage <= nPages )
        {

            uint8_t nTransmit = 18;
            // in the last block only need to
            // transmit the last entries
            if( (nRemain > 0) && (ipage == nPages)  )
            {
                nTransmit = nRemain;
            }

            load_data_from_memory( ipage, nTransmit );

            uint8_t result = transmit_data_with_confirmation(nTransmit);

            // if the checksum doesn't agree, transmit again
            if( result > 0 ) continue;

            ipage++;
        }


    }
        
    if( res1 == 0xd3 && res2==0x55 )
    {
        if( displayTime == 0 ) 
        {
            displayTime = 1;
            struct DateTime dt = read_datetime_from_clock();
            dt = convert_bits_to_datetime( dt );
            display_datetime(dt, 1 );
        }
        else 
        {
            displayTime = 0;
        }
    }

}


int main( void )
{

    DDRB |= 0xc3;
    DDRD |= 0xe0;

    // for Temp/Humid module
    i2c_init();

    // For bluetooth module
    USART_Init(51);

    // For memory chip 
    SPI_init();
    
    // for interrupts
    sei();

    // Reset shift registers
    PORTD &= ~(0x01 << SHIFT_RESET);
    _delay_us(1);
    PORTD |= (0x01 << SHIFT_RESET);
    init_lcd();

    init_clock();

    _delay_ms(5); // ensure 5ms delay between power up and memory operations
    memory_reset();
    // *************
    // write to status register

    uint16_t stat = memory_status();
    display_2bytes( stat);
    _delay_ms(1000);

    _nRecords = recover_records_count();

    //dtInit.year_bits = 0x56;
    //dtInit.day_bits = 0x29;
    //dtInit.month_bits = 0x01;
    //dtInit.date_bits = 0x02;
    //dtInit.hour_bits = 0x16;
    //dtInit.minute_bits = 0x22;
    //dtInit.second_bits = 0x22;

    //write_datetime_to_clock( dtInit );

    while(1) {

        //uint8_t vals[2] = {i, 255-i};
        //memory_sector_erase(0x01, 0x01);
        //memory_write_n( 0x01, 0x01, 0x00, vals, 2);

        //uint8_t val = memory_read_byte( 0x01, 0x01, 0x00);
        //display_time( val, i );
        //_delay_ms(500);
        //val = memory_read_byte( 0x01, 0x01, 0x01);
        //display_time( val, i );
        //_delay_ms(500);

        //uint8_t res = memory_register_read(0x01, i);
        //display_bytes( res, i);
        //_delay_ms(500);
        //memory_write_status_register(0x0c);
        //_delay_ms(5);
        //stat = memory_status();
        //display_2bytes( stat);
        //_delay_ms(1000);

        //uint16_t combID = memory_read_id(0x90);
        //display_bytes( 0x00, 0x90 );
        //_delay_ms(1000);
        //display_2bytes( combID );
        //_delay_ms(2000);
        //combID = memory_read_id(0xab);
        //display_bytes( 0x00, 0xab );
        //_delay_ms(1000);
        //display_2bytes( combID );
        //_delay_ms(2000);
        //combID = memory_read_id(0x4b);
        //display_bytes( 0x00, 0x4b );
        //_delay_ms(1000);
        //display_2bytes( combID );
        //_delay_ms(2000);

        //*******************************
        // AHT10 cycle
        i2c_start( aht10_address_write );
        i2c_write( aht10_address_measure );
        i2c_stop();
        _delay_ms(100);

        i2c_start( aht10_address_read );
        uint8_t status = i2c_read_ack();
        uint8_t hHigh = i2c_read_ack();
        uint8_t hLow = i2c_read_ack();
        uint8_t htmix = i2c_read_ack();
        uint8_t tHigh = i2c_read_ack();
        uint8_t tLow = i2c_read_ack();

        uint32_t temperature = ( (uint32_t)(htmix & 0x0f) << 16);
        temperature = temperature | ((uint16_t)tHigh << 8 );
        temperature = temperature | tLow;

        uint32_t humidity = ( (uint32_t)(hHigh) << 12);
        humidity |= ( (uint16_t)(hLow) << 4 );
        humidity |= ( htmix & (0xf0) );

        struct DateTime dt = read_datetime_from_clock();

        tempStore_0[_blockRecords] = (uint8_t)(temperature);
        tempStore_1[_blockRecords] = (uint8_t)(temperature >> 8);
        tempStore_2[_blockRecords] = (uint8_t)(temperature >> 16);
        tempStore_3[_blockRecords] = (uint8_t)(temperature >> 24);
 
        humidStore_0[_blockRecords] = (uint8_t)(humidity);
        humidStore_1[_blockRecords] = (uint8_t)(humidity >> 8);
        humidStore_2[_blockRecords] = (uint8_t)(humidity >> 16);
        humidStore_3[_blockRecords] = (uint8_t)(humidity >> 24);
 
        yearStore[_blockRecords]   = dt.year_bits;
        monthStore[_blockRecords]  = dt.month_bits;
        dateStore[_blockRecords]   = dt.date_bits;
        hourStore[_blockRecords]   = dt.hour_bits;
        minuteStore[_blockRecords] = dt.minute_bits;
        secondStore[_blockRecords] = dt.second_bits;

        
        //increment counters after
        //data are fully loaded
        //so that an interrupt
        //always gets complete data
        //it may however cut off the
        //last entry
        _nRecords++;
        _blockRecords++;


        if( displayTime == 0 )
        {
            float tempVal = ((float)(temperature)*0.000190735) - 50.;
            float humidVal = ((float)(humidity)/10485.76);

            display_tempHumid( tempVal, humidVal );
        }
        else
        {
            dt = convert_bits_to_datetime( dt );
            display_datetime(dt, 1 );
        }

        if( _blockRecords == 18 )
        {
            write_block_to_memory(18);
            _blockRecords = 0;
        }

        _delay_ms(60000);


    }


}

uint8_t transmit_data_with_confirmation(uint8_t nRecords)
{
   uint8_t checkSum = 0x00;
   for( uint8_t i = 0; i < nRecords; ++i )
   {
       uint8_t year   = yearStore[i];
       uint8_t month  = monthStore[i];
       uint8_t date   = dateStore[i];
       uint8_t hour   = hourStore[i];
       uint8_t minute = minuteStore[i];
       uint8_t second = secondStore[i];

       checkSum ^= year;
       checkSum ^= month;
       checkSum ^= date;
       checkSum ^= hour;
       checkSum ^= minute;
       checkSum ^= second;

       uint8_t temp0 = tempStore_0[i];
       uint8_t temp1 = tempStore_1[i];
       uint8_t temp2 = tempStore_2[i];
       uint8_t temp3 = tempStore_3[i];

       uint8_t humid0 = humidStore_0[i];
       uint8_t humid1 = humidStore_1[i];
       uint8_t humid2 = humidStore_2[i];
       uint8_t humid3 = humidStore_3[i];

       checkSum ^= temp0;
       checkSum ^= temp1;
       checkSum ^= temp2;
       checkSum ^= temp3;

       checkSum ^= humid0;
       checkSum ^= humid1;
       checkSum ^= humid2;
       checkSum ^= humid3;

       USART_Transmit( year );
       USART_Transmit( month );
       USART_Transmit( date );
       USART_Transmit( hour );
       USART_Transmit( minute );
       USART_Transmit( second );
       USART_Transmit( temp3 );
       USART_Transmit( temp2 );
       USART_Transmit( temp1 );
       USART_Transmit( temp0 );
       USART_Transmit( humid3 );
       USART_Transmit( humid2 );
       USART_Transmit( humid1 );
       USART_Transmit( humid0 );

   }

   USART_Transmit( checkSum );

   uint8_t received_checksum = USART_Receive();

   return (received_checksum & (~checkSum ) );

}

void write_block_to_memory(uint8_t nEntries)
{
    // do nothing if there are no records
    if( _nRecords <= 0 ) return;

    // each set of 18 records 
    // fills one page
    // move by 16 pages to leave the first
    // sector for metadata
    uint16_t page_address = ((_nRecords-1)/18) + 16;
    uint16_t sector_address = page_address >> 4;
    uint8_t block_address = (page_address >> 8);
    uint16_t prev_sector = (page_address - 1) >> 4;
    // if we are at a new sector
    // ensure that it is erased
    if( sector_address > prev_sector )
    {
        memory_sector_erase(block_address, page_address);
    }

    uint8_t bytes[256];
    for( int i = 0; i < nEntries; i++ )
    {
        int baseLoc = i*14;
        bytes[baseLoc+0] = yearStore[i];
        bytes[baseLoc+1] = monthStore[i];
        bytes[baseLoc+2] = dateStore[i];
        bytes[baseLoc+3] = hourStore[i];
        bytes[baseLoc+4] = minuteStore[i];
        bytes[baseLoc+5] = secondStore[i];
        bytes[baseLoc+6] = tempStore_3[i];
        bytes[baseLoc+7] = tempStore_2[i];
        bytes[baseLoc+8] = tempStore_1[i];
        bytes[baseLoc+9] = tempStore_0[i];

        bytes[baseLoc+10]  = humidStore_3[i];
        bytes[baseLoc+11]  = humidStore_2[i];
        bytes[baseLoc+12]  = humidStore_1[i];
        bytes[baseLoc+13]  = humidStore_0[i];
    }
    memory_write_n( block_address, page_address, 0x00, bytes, 14*nEntries );

    // erase first sector and
    // write new records count
    memory_sector_erase( 0x00, 0x00 );
    uint8_t recordsBtyes[2];
    recordsBtyes[0] = (uint8_t)(_nRecords >> 8);
    recordsBtyes[1] = (uint8_t)(_nRecords);
    memory_write_n( 0x00, 0x00, 0x00, recordsBtyes, 2);

}

void load_data_from_memory( uint16_t page, uint8_t nEntries)
{
    uint8_t blockAddr = (uint8_t)(page >> 8);
    uint8_t pageAddr = (uint8_t)page;
    uint8_t data[256];
    memory_read_n( blockAddr, pageAddr, 0x00, data, nEntries*14 );
    for( int ient = 0; ient < nEntries; ++ient )
    {
        int baseEnt = ient*14;
        yearStore[ient] = data[baseEnt+0];
        monthStore[ient] = data[baseEnt+1];
        dateStore[ient] = data[baseEnt+2];
        hourStore[ient] = data[baseEnt+3];
        minuteStore[ient] = data[baseEnt+4];
        secondStore[ient] = data[baseEnt+5];

        tempStore_3[ient] = data[baseEnt+6];
        tempStore_2[ient] = data[baseEnt+7];
        tempStore_1[ient] = data[baseEnt+8];
        tempStore_0[ient] = data[baseEnt+9];

        humidStore_3[ient] = data[baseEnt+10];
        humidStore_2[ient] = data[baseEnt+11];
        humidStore_1[ient] = data[baseEnt+12];
        humidStore_0[ient] = data[baseEnt+13];

    }


}
uint16_t recover_records_count(void)
{
    uint8_t records_bytes[2];
    memory_read_n( 0x00, 0x00, 0x00, records_bytes, 2);

    uint16_t records = (uint16_t)(records_bytes[0]) << 8;
    records |= (uint16_t)(records_bytes[1]);

    return records;
}




void SPI_init(void)
{
    // set CS, MOSI and SCK to output
    SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // enable SPI, set as master, and clock to fosc/128
    //SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR1) | (1 << SPR0);
    //SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA);
    //SPCR = (1 << SPE) | (1 << MSTR) ;

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

void USART_Init( uint16_t ubrr)
{
    /*Set baud rate */
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;
    //Enable receiver and transmitter, recieve interrupt */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = (3<<UCSZ00);
}
uint8_t USART_Receive( void )
{
    return UDR0;
}

void USART_Transmit( unsigned char data )
{
    //Wait for empty transmit buffer
    while ( !( UCSR0A & (1<<UDRE0)) );

    UDR0 = data;
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
void lcd_display(uint8_t val)
{
    PORTB = PORTB | ( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    write_shift_byte(val);
    write_to_lcd();
    _delay_us( 50 );
    
}

void lcd_right(void)
{
    PORTB = PORTB & ~( 0x1 << LCD_RS );
    PORTB = PORTB & ~( 0x1 << LCD_RW );
    write_shift_byte( 0xc0 );
    write_to_lcd();
    _delay_us( 50 );
}

void display_number_split( int thousands, int hundreds, int tens, int ones, int tenths, int hundredths )
{

    if( thousands >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[thousands]));
        lcd_display(pat);
    }
    if( hundreds >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[hundreds]));
        lcd_display(pat);
    }

    if( tens >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[tens]));
        lcd_display(pat);
    }
    if( ones >= 0 )
    {
        uint8_t pat = pgm_read_word(&(number_map[ones]));
        lcd_display(pat);
    }

    if( tenths >= 0 && hundredths >= 0 ) 
    {

        lcd_display(0x2e);

        uint8_t pat = pgm_read_word(&(number_map[tenths]));
        lcd_display(pat);

        pat = pgm_read_word(&(number_map[hundredths]));
        lcd_display(pat);
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

        lcd_display( pat );
    }

    lcd_right();

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

        lcd_display(pat);
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

        lcd_display(pat);
    }

    lcd_right();

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

        lcd_display(pat);
    }
}

void display_number( float number, uint8_t padding) {

    int thousands = number/1000;
    float diff_thou = number - thousands*1000;
    int hundreds = diff_thou/100;
    float diff_hund = diff_thou - hundreds*100;
    int tens = diff_hund/10;
    float diff_tens = diff_hund - tens*10;
    int ones = diff_tens;
    float diff_ones = diff_tens - ones;
    int tenths = diff_ones*10;
    float diff_tenths = diff_ones - tenths*0.1;
    int hundredths = diff_tenths*100;

    if( thousands == 0) { 
        thousands = -1;
        if( hundreds == 0) { 
            hundreds = -1;
            if( tens == 0 ) { 
                tens = -1;
                if( ones == 0 ) {
                    ones = -1;
                }
            }
        }
    }

    if ( padding > 0 )
    {
        if( ones < 0 )
        {
            ones = 0;
        }
        padding--;
        if( padding > 0 )
        {
            if( tens < 0 )
            {
                tens = 0;
            }
            padding--;

            if( padding > 0 )
            {
                if( hundreds < 0)
                {
                    hundreds = 0;
                }
                padding--;
                if( padding > 0 && thousands < 0)
                {
                    thousands = 0;
                }
            }
        }
    }

    if( tenths == 0 && hundredths == 0 ) {
        tenths = -1;
        hundredths = -1;
    }
    display_number_split( thousands, hundreds, tens, ones, tenths, hundredths );

}    

void display_time( uint8_t mins, uint8_t secs)
{

    // clear display 
    lcd_clear();

    // return to home
    lcd_home();
    display_number( mins, 2 );

    lcd_display(0x3a);

    display_number( secs, 2 );
}

void display_datetime( struct DateTime dt, uint8_t mode )
{

    lcd_clear();

    lcd_home();

    if( mode == 0 )
    {
        // eg 2020/10/09 03:30
        display_number(dt.year, 4);

        lcd_display(0x2f);

        display_number(dt.month, 2);

        lcd_display(0x2f);

        lcd_right();

        display_number(dt.date, 2);

        lcd_display(0x20);

        display_number(dt.hour, 2);

        lcd_display(0x3a);

        display_number(dt.minute, 2);
    }
    else 
    {
        // eg 10/09 03:30:30
        display_number( dt.month, 2 );

        lcd_display(0x2f);

        display_number( dt.date, 2 );

        lcd_display(0x20);

        display_number( dt.hour, 2 );

        lcd_right();

        lcd_display(0x3a);

        display_number( dt.minute, 2 );

        lcd_display(0x3a);

        display_number( dt.second, 2);

    }
}

void display_tempHumid( float temp, float humid )
{
    lcd_clear();
    lcd_home();

    lcd_display(0x54);
    lcd_display(0x3a);
    display_number(temp, 0);
    lcd_right();
    lcd_display(0x48);
    lcd_display(0x3a);
    display_number(humid, 0);

}

void init_lcd( void )
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
        if( byte & (0x1 << i ) )
        {
            PORTC |= (0x1 << CLOCK_IO );
        }
        else 
        {
            PORTC &= ~(0x1 << CLOCK_IO);
        }

        //clock high
        PORTC |= (0x1 << CLOCK_SCLK);
        //clock low
        PORTC &= ~(0x1 << CLOCK_SCLK);
        // data low 
        PORTC &= ~(0x1 << CLOCK_IO );

    }
}

void clock_enable()
{
    PORTC |= (0x1 << CLOCK_CE);
}

void clock_disable()
{
    PORTC &= ~(0x1 << CLOCK_CE);
}

void clock_io_output(void)
{
    DDRC |= (0x1 << CLOCK_IO);
}

void clock_io_input(void)
{
    DDRC &= ~(0x1 << CLOCK_IO);
}

uint8_t byte_from_clock( void )
{

    uint8_t result = 0;

    for (int i = 0; i < 8; i++) 
    {
        if (PINC & (0x1<<CLOCK_IO))
        {
            result |= (0x01 << i );
        }
        PORTC |= (0x1 << CLOCK_SCLK);
        PORTC &= ~(0x1 << CLOCK_SCLK);
    }
    return result;
}

void write_to_clock( uint8_t address, uint8_t val )
{

    clock_io_output();

    clock_enable();

    byte_to_clock( address );
    byte_to_clock( val );

    clock_disable();

}

uint8_t read_from_clock( uint8_t address )
{

    clock_io_output();

    clock_enable();

    byte_to_clock( address );

    clock_io_input();

    uint8_t value = byte_from_clock();

    clock_io_output();

    clock_disable();

    return value;

}

void init_clock(void)
{
    DDRC |= (1 << CLOCK_SCLK) | (1 << CLOCK_CE);

    write_to_clock( 0x8e, 0x40 );

    // ensure that clock is in 24h mode
    write_to_clock( 0x84, 0x00 );

    // write seconds to zero
    write_to_clock( 0x80, 0x00 );
    write_to_clock( 0x82, 0x00 );
}

void write_datetime_to_clock(struct DateTime data)
{
    // ensure that three bits are set to 0
    // chip enable
    data.second &= ~(0x80);
    // 24h clock
    data.hour &= ~(0x80);

    clock_io_output();

    clock_enable();

    byte_to_clock( 0xbe );
    byte_to_clock( data.second_bits );
    byte_to_clock( data.minute_bits );
    byte_to_clock( data.hour_bits );
    byte_to_clock( data.date_bits );
    byte_to_clock( data.month_bits );
    byte_to_clock( data.day_bits );
    byte_to_clock( data.year_bits );
    byte_to_clock( 0x00 ); // write protect is disabled

    clock_disable();

}

struct DateTime convert_bits_to_datetime( struct DateTime dt )
{

    struct DateTime res;
    res.second_bits = dt.second_bits;
    res.minute_bits = dt.minute_bits;
    res.hour_bits = dt.hour_bits;
    res.date_bits = dt.date_bits;
    res.month_bits = dt.month_bits;
    res.day_bits = dt.day_bits;
    res.year_bits = dt.year_bits;

    uint8_t secs_ones = (dt.second_bits & 0x0f);
    uint8_t secs_tens = (dt.second_bits & 0x70) >> 4;

    res.second = secs_tens*10 + secs_ones;

    uint8_t mins_ones = (dt.minute_bits & 0x0f);
    uint8_t mins_tens = (dt.minute_bits & 0x70) >> 4;

    res.minute = mins_tens*10 + mins_ones;

    uint8_t hour_ones = (dt.hour_bits & 0x0f);
    uint8_t hour_tens = (dt.hour_bits & 0x30) >> 4;

    res.hour = hour_tens*10 + hour_ones;

    uint8_t date_ones = (dt.date_bits & 0x0f);
    uint8_t date_tens = (dt.date_bits & 0x30) >> 4;

    res.date = date_tens*10 + date_ones;

    uint8_t month_ones = (dt.month_bits & 0x0f);
    uint8_t month_tens = (dt.month_bits & 0x10) >> 4;

    res.month = month_tens*10 + month_ones;

    uint8_t year_ones = (dt.year_bits & 0x0f);
    uint8_t year_tens = (dt.year_bits & 0xf0) >> 4;

    res.year = 2000 + (uint16_t)(year_tens*10 + year_ones);

    return res;
}
struct DateTime read_datetime_from_clock()
{

    struct DateTime dt;
    clock_io_output();

    clock_enable();

    byte_to_clock( 0xbf );

    clock_io_input();

    dt.second_bits = byte_from_clock();
    dt.minute_bits = byte_from_clock();
    dt.hour_bits = byte_from_clock();
    dt.date_bits = byte_from_clock();
    dt.month_bits = byte_from_clock();
    dt.day_bits = byte_from_clock();
    dt.year_bits = byte_from_clock();

    // read last byte, but its not needed
    byte_from_clock();

    clock_io_output();

    clock_disable();

    return dt;

}

void memory_chip_enable(void)
{
    PORTB &= ~(1 << CS);
    //_delay_us(10);
}

void memory_chip_disable(void)
{
    PORTB |= (1 << CS);
    //_delay_us(10);
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

uint16_t memory_read_id(uint8_t ch)
{
    memory_chip_enable();
    SPI_masterTransmit( ch ); //ID command
    SPI_masterTransmit( 0x00 ); // need 3 dummy bytes
    SPI_masterTransmit( 0x00 );
    SPI_masterTransmit( 0x00 );

    uint8_t manID = SPI_masterReceive(); 
    uint8_t devID = SPI_masterReceive(); 

    memory_chip_disable();

    uint16_t combID = ((uint16_t)manID << 8 );
    combID |= devID;

    return combID;

}


void memory_write_page(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t bytes[256] )
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

void memory_write_byte(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t byte )
{
    memory_enable_write();

    memory_chip_enable();

    SPI_masterTransmit( 0x02 ); //page write
    SPI_masterTransmit( addr_high ); 
    SPI_masterTransmit( addr_low ); 
    SPI_masterTransmit( addr_byte ); 
    SPI_masterTransmit( byte );

    memory_chip_disable();

    _delay_ms(3);
}

void memory_write_n(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t* bytes, int nEntries )
{
    memory_enable_write();

    memory_chip_enable();

    SPI_masterTransmit( 0x02 ); //page write
    SPI_masterTransmit( addr_high ); 
    SPI_masterTransmit( addr_low ); 
    SPI_masterTransmit( addr_byte ); 
    for( int i = 0; i < nEntries; ++i ) 
    {
        SPI_masterTransmit( bytes[i] ); 
    }

    memory_chip_disable();

    _delay_ms(3);
}

uint8_t memory_read_byte( uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte )
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

void memory_read_n( uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t *results, uint16_t nEntries )
{
    memory_chip_enable();

    SPI_masterTransmit( 0x03 ); //read
    SPI_masterTransmit( addr_high ); 
    SPI_masterTransmit( addr_low ); 
    SPI_masterTransmit( addr_byte ); 
    for( int i = 0; i < nEntries; ++i )
    {
        uint8_t res = SPI_masterReceive(); 
        results[i] = res;
    }
    memory_chip_disable();

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
    _delay_ms(1);
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

    //uint16_t stat = memory_status();
    //display_2bytes( stat);
    //_delay_ms(2000);

    memory_chip_enable();
    SPI_masterTransmit( 0x60 ); //chip erase
    memory_chip_disable();

    //stat = memory_status();
    //display_2bytes( stat);
    //_delay_ms(2000);
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


void memory_block_erase(uint8_t block)
{
    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0xd8 ); //block erase
    SPI_masterTransmit( block );
    SPI_masterTransmit( 0x00 );
    SPI_masterTransmit( 0x00 );
    memory_chip_disable();
    _delay_ms(2000);

}



void memory_reset(void)
{
    memory_chip_enable();
    SPI_masterTransmit( 0x66 ); //enable reset
    memory_chip_disable();

    memory_chip_enable();
    SPI_masterTransmit( 0x99 ); //reset
    memory_chip_disable();
    _delay_ms(5);
}

void memory_register_erase(uint8_t which)
{
    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0x44 ); //register erase
    SPI_masterTransmit( 0x00 );
    SPI_masterTransmit( which << 4 );
    SPI_masterTransmit( 0x00 );
    memory_chip_disable();

    _delay_ms(5);
}

void memory_register_write(uint8_t which, uint8_t reg, uint8_t byte)
{
    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0x42 ); //register write
    SPI_masterTransmit( 0x00 );
    SPI_masterTransmit( which << 4 );
    SPI_masterTransmit( reg );
    SPI_masterTransmit( byte );

    memory_chip_disable();
    _delay_ms(5);

}

uint8_t memory_register_read(uint8_t which, uint8_t reg)
{
    memory_chip_enable();
    SPI_masterTransmit( 0x48 ); //register read
    SPI_masterTransmit( 0x00 );
    SPI_masterTransmit( which << 4 );
    SPI_masterTransmit( reg );
    uint8_t res = SPI_masterReceive( );

    memory_chip_disable();
    return res;
}

void memory_write_status_register(uint8_t val)
{
    memory_enable_write();

    memory_chip_enable();
    SPI_masterTransmit( 0x01 ); //write status register
    SPI_masterTransmit( val ); 
    SPI_masterTransmit( 0x00 ); 
    memory_chip_disable();

}
