#ifndef _SERIAL_H_
#define _SERIAL_H_

void serial_setup(char * rx_buffer, uint16_t max_rx_size, bool& rx_flag, bool& tx_complete);
void serial_enable_interrupts(bool enable);

void serial_write(char * p);
void serial_write(char const * p);
void serial_write(char);

uint8_t serial_rx_length();
void serial_rx_reset();

SerialBuffer& serial_buffer();


#endif