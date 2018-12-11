#include <avr/io.h>
#include <util/delay.h>

void delay_ms(int ms)
{
    while (ms--) {
        _delay_us(1000);  // one millisecond
    }
}
int main(void)
{
  DDRB = 0x18;
  DDRC = 0x00;
  PORTC = 0x01;

  int delay = 100;
  bool button_latch = 0;
  bool target=0;
  while (1) {
    if((PINC & 0x01)==0) { 
      if (button_latch==0 ) {
        button_latch = 1;
	target = !target;
        //delay += 100;
        //if( delay > 800 ) delay = 100;
      }
    }
    else {
      button_latch = 0;
    }
    PORTB = 0x00;
    delay_ms(delay);
    if( target == 0 ) PORTB = 0x08; 
    if( target == 1 ) PORTB = 0x10;
    delay_ms(delay);
  }
}

