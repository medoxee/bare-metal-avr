#include "sht31_driver.h"
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdint.h>

#define	TWBR		0x48
#define	SHT_ADDR	0x44
#define	WBIT		0b0
#define	RBIT		0b1

uint8_t	sht31_interrupt_handler(uint8_t	status)
{
	TWCR |= (1 << TWINT) | (1 << TWEN);
	while (TWCR & (1 << TWINT));
	if (TW_STATUS != status)
		return 1;
	return 0;
}

void	sht31_send_condition(uint8_t	condition)
{	
	TWCR |= (1 << condition) | (1 << TWINT) | (1 << TWEN);
	while (TWCR & (1 << TWINT));
}

int8_t	sht31_init(void)
{
	/* set prescaller to 0b00.
	 * set & send START condition.
	 * wait until it is sent.
	 * reset TWSTA and check if START is transmitted.
	 * if not, flag an error.
	 * create i2c addr + W bit then send it, then check ACK.
	 */
	TWSR &= 0xfc;
	sht31_send_condition(TWSTA);
	if (sht31_interrupt_handler(TW_START))
		return -1;
	TWCR |= ~(1 << TWSTA);
	TWDR = (SHT_ADDR << 1) | WBIT;
	if (sht31_interrupt_handler(TW_MT_SLA_ACK));
		return -2;
	TWDR = 0x24;
	if (sht31_interrupt_handler(TW_MT_SLA_ACK))
		return -3;
	TWDR = 0x0b;
	if (sht31_interrupt_handler(TW_MT_SLA_ACK))
		return -4;
	sht31_send_condition(TWSTO);
	if (sht31_interrupt_handler(TW_STOP))
		return -5;
	_delay_ms(8);
	return 0;
}
