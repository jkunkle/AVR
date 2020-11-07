
struct DateTime{
    uint16_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t year_bits;
    uint8_t day_bits;
    uint8_t month_bits;
    uint8_t date_bits;
    uint8_t hour_bits;
    uint8_t minute_bits;
    uint8_t second_bits;

};


uint8_t transmit_data_with_confirmation(uint8_t start, uint8_t end);
void write_block_to_memory(uint8_t nEntries);
void load_data_from_memory( uint16_t ipage, uint8_t nEntries );
uint16_t recover_records_count(void);

// SPI functions (used for memory)
void SPI_init(void);
uint8_t SPI_masterReceive(void);
void SPI_masterTransmit(uint8_t data);

// USART ( for bluetooth)
void USART_Init( uint16_t ubrr);
uint8_t USART_Receive( void );
void USART_Transmit( unsigned char data );

// shift register (for LCD)
void write_shift_byte( uint8_t data );

// clock read/write
void byte_to_clock( uint8_t byte );
uint8_t byte_from_clock( void );
void clock_enable(void);
void clock_disable(void);
void clock_io_output(void);
void clock_io_input(void);

//*******************
// memory chip
//
// chip enable
void memory_chip_enable(void);
void memory_chip_disable(void);

void memory_enable_write(void);
void memory_disable_write(void);
uint16_t memory_status(void);
//uint8_t memory_read_id(void);
uint16_t memory_read_id(uint8_t ch);
void memory_write_page(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t bytes[256] );
void memory_write_byte(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t bytes);
void memory_write_n(uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t* bytes, int nEntries);
uint8_t memory_read_byte( uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte );
void memory_read_n( uint8_t addr_high, uint8_t addr_low, uint8_t addr_byte, uint8_t *results, uint16_t nEntries );
void memory_write_status( uint8_t status1, uint8_t status2 );
void memory_chip_erase(void);
void memory_sector_erase(uint8_t block, uint8_t sector);
void memory_block_erase(uint8_t block);
void memory_reset(void);
uint8_t memory_register_read(uint8_t which, uint8_t reg);
void memory_register_write(uint8_t which, uint8_t reg, uint8_t byte);
void memory_register_erase(uint8_t which);
void memory_write_status_register(uint8_t val);

//*******************
//


//*******************
// LCD

void init_lcd( void );
void write_to_lcd( void );
void lcd_clear(void);
void lcd_home(void);
void lcd_display(uint8_t val);
void lcd_right(void);
void display_number_split( int thousands, int hundreds, int tens, int ones, int tenths, int hundredths );
void display_2bytes( uint16_t val );
void display_bytes( uint8_t left, uint8_t right );
void display_number( float number, uint8_t padding);
void display_time( uint8_t mins, uint8_t secs);
void display_datetime( struct DateTime dt, uint8_t mode );
void display_tempHumid( float temp, float humid );
//*******************


//******************
// clock
void init_clock(void);
void write_to_clock( uint8_t address, uint8_t val);
uint8_t read_from_clock( uint8_t address);
void write_datetime_to_clock(struct DateTime data);
struct DateTime read_datetime_from_clock(void);
struct DateTime convert_bits_to_datetime( struct DateTime dt );
