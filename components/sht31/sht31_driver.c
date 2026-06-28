#include "sht31_driver.h"
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdint.h>

#define	TWBR		0x48
#define	SHT_ADDR	0x44
#define	WBIT		0b0
#define	RBIT		0b1
#define	CMD_MSB		0x24
#define	CMD_LSB		0x0b
#define	DATA_SIZE	6

uint8_t	sht31_interrupt_handler(uint8_t	status)
{
	TWCR |= (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	if (TW_STATUS != status)
		return 1;
	return 0;
}

void	sht31_send_condition(uint8_t	condition)
{	
	TWCR |= (1 << condition) | (1 << TWINT) | (1 << TWEN);
	if (condition != TWSTO)
		while (!(TWCR & (1 << TWINT)));
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
	TWCR &= ~(1 << TWSTA);
	TWDR = (SHT_ADDR << 1) | WBIT;
	if (sht31_interrupt_handler(TW_MT_SLA_ACK))
		return -2;
	TWDR = CMD_MSB;
	if (sht31_interrupt_handler(TW_MT_DATA_ACK))
		return -3;
	TWDR = CMD_LSB;
	if (sht31_interrupt_handler(TW_MT_DATA_ACK))
		return -4;
	sht31_send_condition(TWSTO);
	if (sht31_interrupt_handler(TW_STOP))
		return -5;
	TWCR &= ~(1 << TWSTO);
	_delay_ms(8);
	return 0;
}

int8_t	sht31_read_data(void)
{
	uint8_t	data[DATA_SIZE];
	uint8_t	byte_counter;

	sht31_send_condition(TWSTA);
	if (sht31_interrupt_handler(TW_START))
		return	-6;
	TWCR &= ~(1 << TWSTA);
	TWDR = (SHT_ADDR << 1) | RBIT;
	if (sht31_interrupt_handler(TW_MR_SLA_ACK))
		return	-7;
	byte_counter = 0;
	TWCR |= (1 << TWEA);
	while  (1)
	{
		if (byte_counter == 5)
		{
			TWCR &= ~(1 << TWEA);
			if (sht31_interrupt_handler(TW_MR_DATA_NACK))
				return	-8;
			data[byte_counter] = TWDR;
			break;
		}
		if (sht31_interrupt_handler(TW_MR_DATA_ACK))
			return	-9;
		data[byte_counter] = TWDR;
		byte_counter++;
	}
	sht_send_condition(TWSTO);
	TWCR &= ~(1 << TWSTO);
	if (sht31_verif_crc(data[0], data[1]) != data[2])
		return	-11;
	if (sht31_verif_crc(data[3], data[4]) != data[5])
		return	-12;
	return	0;
}

uint8_t	sht31_verif_crc(uint8_t	data_msb, uint8_t	data_lsb)
{
	uint8_t	acc;
	uint8_t	counter;
	uint8_t	mes_data[2];
	uint8_t	xsb_byte;

	mes_data[0] = data_msb;
	mes_data[1] = data_lsb;
	acc = CRC_INIT;
	xsb_byte = 0;
	while (xbs_byte < 2)
	{
		acc ^= mes_data[xsb_byte];
		counter = 8;
		while (counter)
		{
			if (acc >> 7)
				acc ^= CRC_POLY;
			acc = acc << 1;
			counter--;
		}
		xsb_byte++;
	}
	return	acc;
}
