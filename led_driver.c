#include "led_driver.h"
#include <avr/io.h>

void led_init(void)
{
  DDRB |= (1 << PB5);
}

void led_turn_on(void)
{
  PORTB |= (1 << PB5);
}

void led_turn_off(void)
{
  PORTB &= ~(1 << PB5);
}