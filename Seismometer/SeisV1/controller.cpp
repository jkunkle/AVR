#define F_CPU 1000000
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c_master.h"
#include "controller.h"
#include "SD/src/SD.h"

#define SHIFT_RESET PB0
#define SHIFT_COPY PD7
#define SHIFT_ENABLE PD6
#define SHIFT_DATA PD5

#define SCOPE_PORT PB1 // FIXME -- update with correct value

uint8_t rtc_device_address = 0x68;
uint8_t rtc_address_seconds = 0x00;
uint8_t rtc_address_minutes = 0x01;
uint8_t rtc_address_hours = 0x02;
uint8_t rtc_address_date = 0x04;
uint8_t rtc_address_month = 0x05;
uint8_t rtc_address_year = 0x06;
uint8_t rtc_address_am1 = 0x07;
uint8_t rtc_address_am2 = 0x08;
uint8_t rtc_address_am3 = 0x09;
uint8_t rtc_address_am4 = 0x0a;
uint8_t rtc_address_conrol = 0x0e;

uint8_t mag_device_addr = 0x22;
uint8_t mag_addr_config1 = 0x00;
uint8_t mag_addr_config2 = 0x01;
uint8_t mag_addr_conf_sensor  = 0x02;
uint8_t mag_config_rd_mode = 0x01;
uint8_t mag_config_samp_rate_4k = 0x0c; // 4.4k sample rate (8x average)
uint8_t mag_config_samp_rate_2k = 0x10; // 2.4k sample rate (16x average)
uint8_t mag_config_cont_samp = 0x02; // continuously sample
uint8_t mag_config_chan = 0x40; // enable Z axis only
                                
uint16_t mag_range_min = 1000; // FIXME 
uint16_t mag_range_up = 2000; // FIXME expect values between 1000 and 3000

uint8_t pwm_range_max = 255;
uint8_t pwm_range_min = 0;

// default data rate to 40 Hz
volatile uint8_t data_rate_hz = 40;

uint16_t fast_read_rate = 4000;
// must be updated if data rate changes
uint16_t fast_mode_write_thresh;

volatile uint8_t mode=1; // default to fast mode
                         
// count data writes as a timer for
// oscilloscope timeout
volatile uint16_t scope_timeout_count=0;
volatile uint16_t fast_read_count=0;
uint8_t scope_timeout_sec=120; // 2 minutes timeout
// must be updated if data rate changes
uint8_t scope_timeout_max;
                              
// Holds the current time
struct DateTime cur_dt;

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

void convert_bits_to_datetime( struct DateTime *dt )
{

    uint8_t secs_ones = (dt->second_bits & 0x0f);
    uint8_t secs_tens = (dt->second_bits & 0x70) >> 4;

    dt->second = secs_tens*10 + secs_ones;

    uint8_t mins_ones = (dt->minute_bits & 0x0f);
    uint8_t mins_tens = (dt->minute_bits & 0x70) >> 4;

    dt->minute = mins_tens*10 + mins_ones;

    uint8_t hour_ones = (dt->hour_bits & 0x0f);
    uint8_t hour_tens = (dt->hour_bits & 0x30) >> 4;

    dt->hour = hour_tens*10 + hour_ones;

    uint8_t date_ones = (dt->date_bits & 0x0f);
    uint8_t date_tens = (dt->date_bits & 0x30) >> 4;

    dt->date = date_tens*10 + date_ones;

    uint8_t month_ones = (dt->month_bits & 0x0f);
    uint8_t month_tens = (dt->month_bits & 0x10) >> 4;

    dt->month = month_tens*10 + month_ones;

    uint8_t year_ones = (dt->year_bits & 0x0f);
    uint8_t year_tens = (dt->year_bits & 0xf0) >> 4;

    uint8_t century = (dt->month_bits & 0x80);

    dt->year = 2000 + (uint16_t)(century*100 + year_tens*10 + year_ones);

}

void convert_datetime_to_bits(struct DateTime *dt)
{

    // seconds (00–59): bits 0–3 ones, 4–6 tens
    dt->second_bits =
        ((dt->second / 10) << 4) |
        (dt->second % 10);

    // minutes (00–59)
    dt->minute_bits =
        ((dt->minute / 10) << 4) |
        (dt->minute % 10);

    // hours (00–23): bits 0–3 ones, 4–5 tens
    dt->hour_bits =
        ((dt->hour / 10) << 4) |
        (dt->hour % 10);

    // date (01–31)
    dt->date_bits =
        ((dt->date / 10) << 4) |
        (dt->date % 10);

    // month (01–12): bits 0–3 ones, bit 4 tens
    dt->month_bits =
        ((dt->month / 10) << 4) |
        (dt->month % 10);

    // year stored as offset from 2000
    uint16_t y = dt->year - 2000;
    dt->month_bits = dt->month_bits | ((y/100) << 7);
    dt->year_bits =
        ((y / 10) << 4) |
        (y % 10);

}

void configure_alarm(void)
{

    i2c_start(rtc_device_address);

    i2c_write(rtc_address_conrol);
    // enable interrupts on alarm1
    i2c_write(0x05);
    i2c_stop();

    // set alarm 1 to interrupt every second
    i2c_start(rtc_device_address);
    i2c_write(rtc_address_am1);
    i2c_write(0x80);
    i2c_write(0x80);
    i2c_write(0x80);
    i2c_write(0x80);
    i2c_stop();

}

void configure_mag_sensor(void)
{

    i2c_start(mag_device_addr);
    i2c_write(mag_addr_config1); //1st device config register
    i2c_write(mag_config_rd_mode | mag_config_samp_rate_4k); 
    i2c_write(mag_config_cont_samp); //2nd device config register
    i2c_write(mag_config_chan); //1st sample config register
    i2c_stop(); // no further config needed
}


uint16_t read_mag_sensor(void)
{
    i2c_start(mag_device_addr | 0x01); // set the read bit
    uint8_t mag_high = i2c_read_ack();
    uint8_t mag_low = i2c_read_ack();
    uint8_t conv_status = i2c_read_nack();
    i2c_stop();

    uint16_t mag_val = (uint16_t)(mag_high << 8) | mag_low;

    return mag_val;
}

void read_time(struct DateTime *dt)
{
    // make sure the register pointer 
    // points to the 0th register
    i2c_start(rtc_device_address);
    i2c_write(rtc_address_seconds);
    i2c_stop();

    // R/W bit to 1 to enable reading
    i2c_start(rtc_device_address | 0x01);
    dt->second_bits = i2c_read_ack();
    dt->minute_bits = i2c_read_ack();
    dt->hour_bits = i2c_read_ack();
    uint8_t nu = i2c_read_ack(); // do not use DoW
    dt->date_bits = i2c_read_ack();
    dt->month_bits = i2c_read_ack();
    dt->year_bits = i2c_read_nack();

    convert_bits_to_datetime(dt);

}
                                  

void write_time(struct DateTime dt)
{

    i2c_start(rtc_device_address);
    // start writing at the first address
    // The DS3231 automatically 
    // advnaces to the next address
    // at each write
    i2c_write(rtc_address_seconds);
    
    i2c_write(dt.second_bits);
    i2c_write(dt.minute_bits);
    i2c_write(dt.hour_bits);
    // we don't care about the DoW
    // set it to 0x01 so that 
    // we can set bytes sequentially
    i2c_write(0x01);
    i2c_write(dt.date_bits);
    i2c_write(dt.month_bits);
    i2c_write(dt.year_bits);
    i2c_stop();
}

void disable_adc(void) {

    ACSR = 0x80;
}

void enable_scope(void) {
    
    PORTB |= (0x1 << SCOPE_PORT);
    scope_timeout_count = 0;

}

void disable_scope(void) {
    
    PORTB &= ~(0x1 << SCOPE_PORT);

}


// Interrupt from RTC module
ISR(INT0_vect){

    // rather than reading the RTC
    // every second, just update the 
    // internal time until seconds count
    // reaches 60.  Only in this case, 
    // do a full read and update
    cur_dt.second = cur_dt.second + 1;

    if( cur_dt.second >= 60)
    {
        read_time(&cur_dt);
    }

}

//void configure_timer_fast(rate)
//{
//    // FIXME set timer level and or prescale
//}

// Interrupt from external button
ISR(INT1_vect){

    enable_scope();

}

ISR( TIMER0_COMPA_vect ){ //FIXME correct timer and vector

    if( mode == 1 ) { // Fast mode
      uint16_t mag_val = read_mag_sensor();
      fast_read_count += 1;

      if(fast_read_count >= fast_mode_write_thresh) {
          //write_data(cur_dt, mag_val);
          fast_read_count = 0;
          scope_timeout_count += 1;
      }

      if(scope_timeout_count >= scope_timeout_max)
      {
          disable_scope();
      }
    }
    if( mode == 0 ) { // slow mode
    }




}
int main(void)
{

  fast_mode_write_thresh = fast_read_rate/data_rate_hz;
  scope_timeout_max = scope_timeout_sec*data_rate_hz;
  //DDRD |= (1 << PD5 ) | (1 << PD6 ) | (1 << PD7) | (0 << PD2);
  //
  // Enable output ports for LED 7-segment display
  DDRD |= (1 << PD5 ) | (1 << PD6 ) | (1 << PD7) ;
  DDRB |= ( 1 << PB0 ); 

  // enable interrupts
  sei();

  //PORTD |= 0<<PD2;

  // Timers -- need to check again
  //TCCR0B = ((1 << CS02) | (1 << CS00));
  //TCCR1B = ((1 << CS12) | (1 << CS10));
  //TCCR1B = ((1 << CS12));

  reset_shift();

  configure_alarm();
  configure_mag_sensor();

  disable_adc();

  enable_scope();

  _delay_us(500);

  // test datetime
  struct DateTime dt;
  dt.year = 2026;
  dt.month=2;
  dt.date=1;
  dt.hour=22;
  dt.minute=50;
  dt.second=0;
  // convert time to bits
  convert_datetime_to_bits(&dt);

  // write to rtc module
  write_time(dt);


  while(1) {

      _delay_us(1000);

      read_time(&dt);

      display_number(dt.year, 100);
      _delay_us(1000);
      display_number(dt.month, 100);
      _delay_us(1000);
      display_number(dt.date, 100);
      _delay_us(1000);
      display_number(dt.hour, 100);
      _delay_us(1000);
      display_number(dt.minute, 100);
      _delay_us(1000);
      display_number(dt.second, 100);
      _delay_us(1000);

      display_number(2197, 100);
  }
}

