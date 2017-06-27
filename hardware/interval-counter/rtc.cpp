#include <Wire.h>

#include "rtc.h"

static const uint8_t DS1307_I2C_ADDRESS = 0x68;

void rtc_setup()
{
	// Configure for 32768Hz output
	Wire.beginTransmission(DS1307_I2C_ADDRESS);
	Wire.write((uint8_t)0x07);
	Wire.write((uint8_t)0b000100011);
	Wire.endTransmission();
}

