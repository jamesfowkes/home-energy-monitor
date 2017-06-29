#include <avr/io.h>

#include <stdint.h>
#include <stdlib.h>

#include "rtc.h"

#include "lib_i2c_common.h"
#include "lib_i2c_config.h"

static const uint8_t DS1307_I2C_ADDRESS = 0x68;

static uint8_t s_tx_buffer[2];

static I2C_TRANSFER_DATA s_transfer_data;

void rtc_setup()
{
	I2C_SetPrescaler(64);
	I2C_Init(NULL);

	//// Configure for 32768Hz output
	s_transfer_data.address = DS1307_I2C_ADDRESS;
	s_transfer_data.buffer = s_tx_buffer;
	s_transfer_data.totalBytes = 2;

	PORTB |= (1 << 5);
	s_tx_buffer[0] = 0x07; s_tx_buffer[1] = 0x13;
	I2C_StartMaster(&s_transfer_data, false, false);
	while(I2C_IsActive()) { I2C_Task(); }

	s_tx_buffer[0] = 0x00; s_tx_buffer[1] = 0x00;
	I2C_StartMaster(&s_transfer_data, false, false);
	while(I2C_IsActive()) { I2C_Task(); }

	PORTB &= ~(1 << 5);
}

