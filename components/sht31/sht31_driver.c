#include "sht31_driver.h"
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <stdint.h>

#define	SHT_ADDR	0x44
#define	WBIT		0b0
#define	RBIT		0b1
#define	CMD_MSB		0x24
#define	CMD_LSB		0x0b
#define	DATA_SIZE	6
#define	CRC_INIT 	0xff
#define	CRC_POLY	0x31
#define	CRC_ERR 	0xcc

static	void	sht31_convert(int16_t	*scaled_data, uint16_t	temp_msb,
		uint16_t	temp_lsb, uint16_t	hum_msb,
		uint16_t	hum_lsb)
{
	uint16_t	raw_temp;
	uint16_t	raw_hum;

	raw_temp = (temp_msb << 8) | temp_lsb;
	raw_hum = (hum_msb << 8) | hum_lsb;
	scaled_data[0] = -45 + ((175 * (uint32_t)raw_temp) / 65535);
	scaled_data[1] = (100 * (uint32_t)raw_hum) / 65535;
}

static	uint8_t	sht31_verif_crc(uint8_t	data_msb, uint8_t	data_lsb)
{
	uint8_t	acc;
	uint8_t	counter;
	uint8_t	mes_data[2];
	uint8_t	xsb_byte;
	uint8_t	msb_bit;

	mes_data[0] = data_msb;
	mes_data[1] = data_lsb;
	acc = CRC_INIT;
	xsb_byte = 0;
	while (xsb_byte < 2)
	{
		acc ^= mes_data[xsb_byte];
		counter = 8;
		while (counter)
		{
			msb_bit = acc >> 7;
			acc = acc << 1;
			if (msb_bit)
				acc ^= CRC_POLY;
			counter--;
		}
		xsb_byte++;
	}
	return	acc;
}

static	uint8_t	sht31_interrupt_handler(uint8_t	status_code)
{
	TWCR |= (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	if (TW_STATUS != status_code)
		return 1;
	return 0;
}

static	void	sht31_send_condition(uint8_t	condition)
{	
	TWCR = (1 << condition) | (1 << TWINT) | (1 << TWEN);
	if (condition != TWSTO)
		while (!(TWCR & (1 << TWINT)));
}

uint8_t	sht31_init(void)
{
	/* 
	 * set bit rate to 0x48 (calculated from formula of sensor manifacturer)
	 * set prescaller to 0b00.
	 * set & send START condition.
	 * wait until it is sent.
	 * reset TWSTA and check if START is transmitted.
	 * if not, flag an error.
	 * create i2c addr + W bit then send it, then check ACK.
	 */
	TWBR = 0x48;
	TWSR &= 0xfc;
	sht31_send_condition(TWSTA);
	sht31_send_condition(TWSTA);
	if (sht31_intrrupt_handler(TW_START)
		return	0x01;
	TWCR &= ~(1 << TWSTA);
	TWDR = (SHT_ADDR << 1) | WBIT;
	if (sht31_interrupt_handler(TW_MT_SLA_ACK))
		return 0x02;
	TWDR = CMD_MSB;
	if (sht31_interrupt_handler(TW_MT_DATA_ACK))
		return 0x03;
	TWDR = CMD_LSB;
	if (sht31_interrupt_handler(TW_MT_DATA_ACK))
		return 0x04;
	sht31_send_condition(TWSTO);
	_delay_ms(8);
	return 0;
}

uint8_t	sht31_read_data(int16_t	*scaled_data)
{
	uint8_t	raw_data[DATA_SIZE];
	uint8_t	byte_counter;

	sht31_send_condition(TWSTA);
	if (sht31_intrrupt_handler(TW_START)
		return	0x05;
	TWCR &= ~(1 << TWSTA);
	TWDR = (SHT_ADDR << 1) | RBIT;
	if (sht31_interrupt_handler(TW_MR_SLA_ACK))
		return	0x06;
	byte_counter = 0;
	TWCR |= (1 << TWEA);
	while  (1)
	{
		if (byte_counter == 5)
		{
			TWCR &= ~(1 << TWEA);
			if (sht31_interrupt_handler(TW_MR_DATA_NACK))
				return	0x07;
			raw_data[byte_counter] = TWDR;
			break;
		}
		if (sht31_interrupt_handler(TW_MR_DATA_ACK))
			return	0x08;
		raw_data[byte_counter] = TWDR;
		byte_counter++;
	}
	sht31_send_condition(TWSTO);
	if (sht31_verif_crc(raw_data[0], raw_data[1]) != raw_data[2])
		return	0x09;
	if (sht31_verif_crc(raw_data[3], raw_data[4]) != raw_data[5])
		return	0x0A;
	sht31_convert(scaled_data, raw_data[0], raw_data[1], raw_data[3],
			raw_data[4]);
	return	0;
}
