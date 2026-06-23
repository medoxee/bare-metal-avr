#include "led_driver.h"
#include <util/delay.h>

#define F_CLK 16000000UL

void main(void)
{
  /* Initilizing pin D13 as output */
  led_init();

  /* loop */
  while(1)
  {
    led_turn_on();
    _delay_ms(1000);
    led_turn_off();
    _delay_ms(1000);
  }
}