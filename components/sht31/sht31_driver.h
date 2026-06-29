/*
 * 				IN THE NAME OF ALLAH,
 * History	: 26-06-2026
 * Author	: MOHAMED ARRAF -- (www.linkedin.com/in/mohamedarraf).
 * Purpose	: Controlling sht31 sensor.
 * Algorithm	:
 *
 *  # Setting:
 * 	1- The MCU sends a start condition state of I2C protocol by switching
 * 	SDA to LOW before SCL to LOW.
 * 	2- MCU sends 7 bits addresse of slave via SDA line plus bit 0 to WRITE
 * 	data.
 * 	3- Slave sends back an ACK if addresse matches it, if not it leaves SDA
 * 	in a HIGH state.
 * 	4- MCU sends Command MSB 0x24 (Search for Commands of SHT31).
 * 	5- ACK by slave.
 * 	6- MCU sends Command LSB 0x0B.
 * 	7- ACK by slave.
 * 	8- MCU generates a STOP condition.
 * 	9- MCU waits 8 ms for the slave to measure T&H.
 *
 * # Read of data:
 * 	1- MCU generates a START condition.
 * 	2- MCU sends I2C header with bit 1 to READ data.
 * 	3- ACK by slave.
 * 	4- Slave sends 8 bits MSB of Temp data.
 * 	5- ACK by MCU.
 * 	6- Slave sends 8 bits LSB of Temp data.
 * 	7- ACK by MCU.
 * 	8- Slave sends 8 bits as CRC.
 * 	9- ACK by MCU.
 * 	... same thing goes with Humidity data (Last 5 steps).
 * 	10- NACK and STOP conditino sent by MCU after receiving data.
 *
 * # Data processing:
 * 	1- Calculate the CRC Checksum to verify data.
 * 	2- Convert raw data into a physical scale (T: °C, H: %) by using
 * 	the mathematical formula from sensor manifacuterer (Sensirion).
 *
 * Functions	:
 * 	sht31_init	: Initilize sensor and prepare it for measurement.
 * 	sht31_read_data	: Read Temp/Humidity data.
 * 	sht31_verif_crc	: Verify CRC Checksum, if not equal to CRC, return
 * 			error flag CRC_ERR.
 * 	sht31_convert	: Converts raw data to pysical scale data.
 */

#include <stdint.h>

uint8_t	sht31_init(void);
uint8_t	sht31_read_data(void);
uint8_t	sht31_verif_crc(uint8_t	data_msb, uint8_t	data_lsb);
uint16_t	*sht31_convert(uint16_t	temp_msb, uint16_t	temp_lsb,
		uint16_t	hum_msb, uint16_t	hum_lsb);
