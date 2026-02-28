
volatile uint8_t fast_read_flag=0;
uint8_t scope_enabled=1; 
ISR( TIMER1_COMPA_vect ){ //FIXME correct timer and vector

    fast_read_flag += 1;
    write_count += 1;
    button_poll_count += 1;


    // one possibility is to read sensor and
    // update PWM in the interrupt routine
    //uint16_t mag_val = read_mag_sensor();
    //if( scope_enabled ) {
    //  update_pwm(mag_val);
    //}
}

int main(void)
{

    //steup

  while(1) {

      if( fast_read_flag > 0 )
      {
          fast_read_flag = 0;
          uint16_t mag_val = read_mag_sensor();
          if( scope_enabled ) {
              update_pwm(mag_val);
          }
      }
  }
}


