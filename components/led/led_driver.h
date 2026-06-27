/* Purpuse: Controlling the D13 (PB5) pin */
/* Algorithm: Initilizing D13, write HIGH, write LOW */
/* Functions:
              led_init: Initilizes D13 as output.
              led_turn_on: Turn on led in D13.
              led_turn_off: Turn off led in D13.
*/

void led_init(void);
void led_turn_on(void);
void led_turn_off(void);