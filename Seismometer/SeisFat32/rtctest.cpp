#define F_CPU 16000000
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c_master.h"
#include "rtctest.h"

// ATMEGA 328p pins
//
// PB0 : SHIFT RESET
// PB1 : NC
// PB2 : SPI SS
// PB3 : SPI MOSI
// PB4 : SPI MISO
// PB5 : SPI SCK
// PB6 : NC 
// PB7 : NC 
// PC0 : PCINT8 : rotary encoder A
// PC1 : PCINT9 : rotary encoder B
// PC2 : PCINT10 : rotarty encoder button?
// PC3 : I2C SCL
// PC4 : I2C SDA
// PC5 : NC 
// PC6 : NC 
// PD0 : NC 
// PD1 : External Button
// PD2 : INT0 RTC
// PD3 : OC2B
// PD4 : SHIFT ENABLE
// PD5 : SHIFT DATA
// PD6 : SCOPE PIN
// PD7 : SHIFT COPY

#define SHIFT_RESET PB0
#define SHIFT_COPY PD7
#define SHIFT_ENABLE PD4
#define SHIFT_DATA PD5

#define BUTTON_PIN PD1
#define SCOPE_PIN PD6

static volatile uint32_t g_millis = 0;
static volatile uint8_t  sub_ms = 0;   // counts 0.25ms ticks
extern "C" uint32_t millis(void)
{
    uint32_t m;
    uint8_t sreg = SREG;
    cli();
    m = g_millis;
    SREG = sreg;
    return m;
}
#include "ArduinoCompat.h"
#include "AvrSdSpiDriver.h"
#include "SdFat.h"
#include "SdFatConfig.h"

static const uint8_t rtc_device_address = 0xD0;
static const uint8_t rtc_address_seconds = 0x00;
static const uint8_t rtc_address_minutes = 0x01;
static const uint8_t rtc_address_hours = 0x02;
static const uint8_t rtc_address_date = 0x04;
static const uint8_t rtc_address_month = 0x05;
static const uint8_t rtc_address_year = 0x06;
static const uint8_t rtc_address_am1 = 0x07;
static const uint8_t rtc_address_am2 = 0x08;
static const uint8_t rtc_address_am3 = 0x09;
static const uint8_t rtc_address_am4 = 0x0a;
static const uint8_t rtc_address_conrol = 0x0e;
static const uint8_t rtc_address_status = 0x0f;

static const uint8_t mag_device_addr = 0x44;
static const uint8_t mag_addr_config1 = 0x00;
static const uint8_t mag_addr_config2 = 0x01;
static const uint8_t mag_addr_conf_sensor  = 0x02;
static const uint8_t mag_config_rd_mode = 0x01;
static const uint8_t mag_config_samp_rate_4k = 0x0c; // 4.4k sample rate (8x average)
static const uint8_t mag_config_samp_rate_2k = 0x10; // 2.4k sample rate (16x average)
static const uint8_t mag_config_cont_samp = 0x02; // continuously sample
static const uint8_t mag_config_chan = 0x40; // enable Z axis only
                                
uint16_t mag_range_min = 3000; // FIXME 
uint16_t mag_range_max = 15000; // FIXME expect values between 1000 and 3000

uint8_t pwm_range_max = 255;
uint8_t pwm_range_min = 0;

// default data rate to 40 Hz
uint8_t data_rate_hz = 40;

// default PWM value to 0
uint8_t pwm_value = 0;

uint16_t read_rate = 400;
// must be updated if data rate changes

uint8_t mode=0;
                         
// count data writes as a timer for
// oscilloscope timeout
volatile uint16_t scope_timeout_count=0;
uint16_t write_count=0;

volatile uint8_t fast_read_flag=0;
volatile uint8_t write_flag=0;

uint16_t scope_timeout_sec=120; // 2 minutes timeout
uint8_t scope_enabled=1; 
                                
// must be updated if data rate changes
//
uint16_t read_count_thresh=0;
uint16_t scope_timeout_max=0;
uint8_t write_count_thresh=0;
                              
// Holds the current date/time
struct DateTime cur_dt;

SdFat sd;

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

uint8_t reset_dtc_alarm(void)
{
    // Read status
    uint8_t res = 0;
    uint8_t status;
    res |= i2c_start(rtc_device_address);      // 0xD0 write
    res |= i2c_write(rtc_address_status);
    res |= i2c_start(rtc_device_address | 0x01); // 0xD1 read
    status = i2c_read_nack();
    i2c_stop();
    
    // Clear A1F (bit0)
    status &= ~(1 << 0);
    
    // Write status back
    res |= i2c_start(rtc_device_address);
    res |= i2c_write(rtc_address_status);
    res |= i2c_write(status);
    i2c_stop();

    return res;

}

uint8_t configure_dtc_alarm(void)
{

    uint8_t res = 0;
    res |= i2c_start(rtc_device_address);
    // set alarm 1 to interrupt every second
    res |= i2c_write(rtc_address_am1);
    res |= i2c_write(0x80);
    res |= i2c_write(0x80);
    res |= i2c_write(0x80);
    res |= i2c_write(0x80);
    i2c_stop();

    i2c_init();
    res |= i2c_start(rtc_device_address);
    res |= i2c_write(rtc_address_conrol);
    // enable interrupts on alarm1
    res |= i2c_write(0x05);
    i2c_stop();

    res |= reset_dtc_alarm();

    return res;

}


uint8_t configure_mag_sensor(void)
{

    uint8_t res = 0;
    res |= i2c_start(mag_device_addr);
    res |= i2c_write(mag_addr_config1); //1st device config register
    i2c_write(mag_config_rd_mode | mag_config_samp_rate_4k); 
    i2c_write(mag_config_cont_samp); //2nd device config register
    i2c_write(mag_config_chan); // configure to read only Z channel
    i2c_stop(); // no further config needed
                
    return res;
}


uint16_t read_mag_sensor(void)
{
    i2c_start(mag_device_addr | 0x01); // set the read bit
    uint8_t mag_high = i2c_read_ack();
    uint8_t mag_low = i2c_read_ack();
    // conversion status, can be read 
    // if needed
    (void)i2c_read_nack();
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
    (void) i2c_read_ack(); // do not use DoW
    dt->date_bits = i2c_read_ack();
    dt->month_bits = i2c_read_ack();
    dt->year_bits = i2c_read_nack();

    convert_bits_to_datetime(dt);

}
                                  

uint8_t write_time(struct DateTime dt)
{

    uint8_t res = i2c_start(rtc_device_address);
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

    return res;
}

void disable_adc(void) {

    ACSR = 0x80;
}

void setup_scope(void){

    DDRD |= (0x1 << SCOPE_PIN);

}
void enable_scope(void) {
    
    // enable PWM

    // FIXME -- check port
    PORTD |= (0x1 << SCOPE_PIN);
    scope_timeout_count = 0;
    scope_enabled = 1;


}

void disable_scope(void) {
    
    PORTD &= ~(0x1 << SCOPE_PIN);
    // no output on PWM pin
    OCR2B = 0;
    scope_enabled = 0;


}

void enable_read_timer(uint16_t comp_val) 
{

    // disable timers
    TCCR1B = 0;
    TCCR1A = 0;

    // enable timer with prescale of 1
    TCCR1B = (0x1 << CS10) | (0x1 << WGM12);
    // enable output compare match A interrupt
    // Note: Interrupts need to be enabled
    TIMSK1 = (0x1 << OCIE1A);

    OCR1A = comp_val;

}

void update_read_timer(uint16_t comp_val) 
{

    // disable interrupts while setting to avoid
    //
    cli();
    OCR1A = comp_val;
    sei();

}

void enable_pwm_timer() 
{

    // disable timers and interrupts
    TCCR2A = 0;
    TCCR2B = 0;
    TIMSK2 = 0; 

    // enable output port
    DDRD |= (1 << DDD3);

    // enable PWM mode, flip output on compare match
    TCCR2A = (0x1 << COM2B1) | (0x1 << WGM21) | (0x1 << WGM20);
    //TCCR2B = (0x1 << CS20) | (0x1 << CS21) | (0x1 << CS22);
    TCCR2B = (0x1 << CS20);

    // start with no output
    OCR2B = 0;

}

void update_pwm(uint16_t mag_val)
{
      if (mag_val <= mag_range_min) {
        OCR2B = 0;
        return;
    }
    if (mag_val >= mag_range_max) {
        OCR2B = 255;
        return;
    }

    uint16_t x    = (uint16_t)(mag_val - mag_range_min);
    uint16_t span = (uint16_t)(mag_range_max - mag_range_min);

    //display_number(x, 200);
    OCR2B = (uint8_t)(((uint32_t)x * 255u) / span);

    //if( mag_val < mag_range_min ) {
    //    OCR2B = 0;
    //}
    //else {
    //    OCR2B = (uint8_t)(((float)mag_val - (float)mag_range_min)*255)/((float)mag_range_span);
    //}
}

bool sd_init()
{
  // Use SS just as a default cs “id”. If your driver controls CS itself, this can be ignored.
  SdSpiConfig cfg(SS, DEDICATED_SPI, SD_SCK_MHZ(4), &sdSpi);
  return sd.begin(cfg);
}

void set_clock(void)
{
    // enable prescale change
    CLKPR = (0x1 << CLKPCE);
    // prescale bits to 0x1 -> prescale of 2
    //CLKPR = (0x1 << CLKPS0 | 0x1 << CLKPS1);
    CLKPR = (0x1 << CLKPS0);
}


uint16_t update_read_timer_thresh(uint16_t read_rate)
{
    return (uint16_t) ((F_CPU/read_rate) - 1);
}

uint8_t update_write_count_thresh(uint16_t read_rate, uint8_t data_rate)
{
    return (uint8_t)((read_rate/data_rate) -1);
}

uint8_t update_button_poll_thresh(uint16_t read_rate, uint16_t poll_rate )
{
    if( poll_rate > read_rate ){
        return 1;
    }
    return (uint8_t)((read_rate/poll_rate) - 1);
}

uint16_t update_scope_timeout(uint16_t timeout_sec, uint8_t data_rate)
{
    return timeout_sec*(uint16_t)data_rate;
}

void setup_external_interrupts(void)
{
    // enable INT0 on PD2
    // activated on falling edge
    DDRD &= ~(0x1 << DDD2);
    EICRA = (EICRA & ~((0x1<<ISC01) | (0x1<<ISC00))) | (0x1<<ISC01);
    EIFR  |= (0x1 << INTF0);
    EIMSK = (0x1 << INT0 );

    // Enable pin change interrupts
    // on pins PC8,PC9,PC10
    //

}



// Interrupt from RTC module
//ISR(INT0_vect){
//
//    // rather than reading the RTC
//    // every second, just update the 
//    // internal time until seconds count
//    // reaches 60.  Only in this case, 
//    // do a full read and update
//    cur_dt.second = cur_dt.second + 1;
//
//    if( cur_dt.second >= 60)
//    {
//        read_time(&cur_dt);
//    }
//
//    reset_dtc_alarm();
//
//}

// Interrupt from external button
//ISR(INT1_vect){
//
//    enable_scope();
//
//}

ISR( TIMER1_COMPA_vect ){

    fast_read_flag = 1;
    write_count += 1;

    //PIND = (1 << PIND6);
    //uint16_t mag_val = read_mag_sensor();
    //if( scope_enabled ) {
    //  update_pwm(mag_val);
    //}
    // check all counts that are based
    // on the fast read timer
    // if a count matches/exceeds the threshold, 
    // raise the flag, which will be 
    // utilized in the main loop
    if( write_count > write_count_thresh) {
        write_flag = 1;
        write_count = 0;
        scope_timeout_count += 1;
    }
        // 4 ticks = 1ms
    if (++sub_ms >= 4) {
        sub_ms = 0;
        g_millis++;
    }

}

int main(void)
{

  //set_clock();
  //disable_adc();

  // Enable output ports for LED 7-segment display
  DDRD |= (1 << SHIFT_DATA ) | (1 << SHIFT_ENABLE ) | (1 << SHIFT_COPY) | (1 << PD6);
  DDRB |= ( 1 << SHIFT_RESET ); 

  // button push
  DDRD &= ~(1 << DDD1);      // input
  PORTD |= (1 << PORTD1);    // enable internal pull-up

  read_count_thresh = update_read_timer_thresh(read_rate);
  write_count_thresh = update_write_count_thresh(read_rate, data_rate_hz);
  scope_timeout_max = update_scope_timeout(scope_timeout_sec, data_rate_hz);


  i2c_init();

  reset_shift();

  configure_dtc_alarm();
  uint8_t res = configure_mag_sensor();

  //// read at 400 Hz
  enable_read_timer(read_count_thresh);

  //// enable the pwm
  enable_pwm_timer();

  //sd_init();

  //setup_external_interrupts();

  //// enable interrupts
  sei();

  setup_scope();
  enable_scope();
  update_pwm(0);

  _delay_us(500);

  // test datetime
  struct DateTime dt;
  dt.year = 2026;
  dt.month=2;
  dt.date=22;
  dt.hour=14;
  dt.minute=25;
  dt.second=0;
  // convert time to bits
  convert_datetime_to_bits(&dt);

  //write_time(dt);

  //update_pwm(128);
  while(1) {


      //PIND = (1 << PIND6);
      //_delay_us(500);

      if(fast_read_flag)
      {
        fast_read_flag=0;
        int16_t mag_val = read_mag_sensor();
        update_pwm(mag_val);
        if( !(PIND & (0x1 << BUTTON_PIN))){
            enable_scope();
        }
      }
      if( write_flag ){
          write_flag = 0;
          //PIND = (1 << PIND6);
      }
      ////    fast_read_flag = 0;
      ////    //if( scope_enabled ) {
      ////    //    update_pwm(mag_val);
      ////    //}

      ////    if( write_flag ) {
      ////        write_flag = 0;
      ////        PORTD ^= (0x1 << SCOPE_PIN);
      ////        //display_number(mag_val, 100);
      ////    }
      ////    //    // Count for scope timeouts within data writes
      ////    //    // so that it fits in 16 bits
      ////    //    scope_timeout_count += 1;
      ////    //}
      ////    //if( button_poll_flag ) {
      ////    //    button_poll_flag = 0;
      ////    //    if( !(PIND & (0x1 << BUTTON_PIN))){
      ////    //        enable_scope();
      ////    //    }
      ////    //}

      ////}
      if( scope_enabled ) 
      {
          if(scope_timeout_count > scope_timeout_max)
          {
              cli();
              scope_timeout_count = 0;
              sei();
              disable_scope();
          }
      }
  }
}

      //read_time(&dt);
      //display_number(dt.second, 100);
      //display_number(dt.minute, 100);
      //display_number(dt.hour, 100);
      //display_number(dt.date, 100);
      //display_number(dt.month, 100);
      //display_number(dt.year, 100);
      //
