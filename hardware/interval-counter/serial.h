#ifndef _SERIAL_H_
#define _SERIAL_H_

void serial_setup(char * rx_buffer, uint16_t max_rx_size, bool& rx_flag, bool& tx_complete);
void serial_enable_interrupts(bool enable);

void serial_write(char * p);
void serial_write(char const * p);
void serial_write(char c);
void serial_write(unsigned char c);
void serial_write(int16_t i16);
void serial_write(uint16_t u16);
void serial_write(int32_t i32);
void serial_write(uint32_t u32);

uint8_t serial_rx_length();
void serial_rx_reset();

SerialBuffer& serial_buffer();


#endif