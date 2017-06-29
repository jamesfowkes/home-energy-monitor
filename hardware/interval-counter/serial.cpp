/* Platform Includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* C/C++ Includes */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Application Includes */

#include "buffer.h"

/* Defines, constants, typedefs */

#define TARGET_BAUD_RATE (115200)
#define USE_U2X

/*
 * Private Module Functions
 */


static inline void disable_udre_interrupt() { UCSR0B &= ~_BV(UDRIE0); }
static inline void enable_udre_interrupt() { UCSR0B |= _BV(UDRIE0); }

static inline void disable_txc_interrupt() { UCSR0B &= ~_BV(TXCIE0); }
static inline void enable_txc_interrupt() { UCSR0B |= _BV(TXCIE0); }

static const int TX_BUFFER_SIZE = 32;

/*
 * Private Module Variables
 */

static volatile char s_tx_buffer_chars[TX_BUFFER_SIZE];

static volatile uint8_t s_transmit_index = 0;
static volatile uint8_t s_transmit_length = 0;

static volatile bool * sp_rx_flag = NULL;
static volatile bool * sp_tx_complete_flag = NULL;

static SerialBuffer s_rx_buffer;

static char s_local_format_buffer[32];

/*
 * Private Module Functions
 */

static void serial_write(char const * const buffer, int n)
{
	s_transmit_index = 0;
	
	if (n > TX_BUFFER_SIZE)
	{
		s_transmit_length = TX_BUFFER_SIZE;
	}
	else
	{
		s_transmit_length = n;	
	}
		
	memcpy((void *)s_tx_buffer_chars, buffer, s_transmit_length);

	cli();
	UDR0 = s_tx_buffer_chars[s_transmit_index++];
	enable_udre_interrupt();
	enable_txc_interrupt();
	sei();
}

/*
 * Public Module Functions
 */

void serial_write(char * p)
{
	serial_write(p, strlen(p));
}

void serial_write(char const * p)
{
	serial_write(p, strlen(p));
}

void serial_write(char c)
{
	sprintf(s_local_format_buffer, "%c", c);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_write(unsigned char c)
{
	sprintf(s_local_format_buffer, "%c", c);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_write(int16_t i16)
{
	sprintf(s_local_format_buffer, "%d", i16);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_write(uint16_t u16)
{
	sprintf(s_local_format_buffer, "%u", u16);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_write(int32_t i32)
{
	sprintf(s_local_format_buffer, "%ld", i32);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_write(uint32_t u32)
{
	sprintf(s_local_format_buffer, "%lu", u32);
	serial_write(s_local_format_buffer, strlen(s_local_format_buffer));
}

void serial_enable_interrupts(bool enable)
{
	if (enable)
	{
		UCSR0B |= (_BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0));
	}
	else
	{
		UCSR0B &= ~(_BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0));
	}
}

void serial_setup(char * prx_buffer, uint16_t max_rx_size, bool& rx_flag, bool& tx_complete)
{
	sp_rx_flag = &rx_flag;
	sp_tx_complete_flag = &tx_complete;
	
	s_rx_buffer.init(prx_buffer, max_rx_size);

	#ifdef USE_U2X
		UCSR0A |= _BV(U2X0);
		UBRR0 = (F_CPU / (TARGET_BAUD_RATE * 8)) - 1;
	#else
		UBRR0 = (F_CPU / (TARGET_BAUD_RATE * 16)) - 1;
	#endif
}

SerialBuffer& serial_buffer()
{
	return s_rx_buffer;
}

ISR(USART_RX_vect)
{
	s_rx_buffer.add_to_buffer((char)UDR0);
	*sp_rx_flag = true;
}

ISR(USART_UDRE_vect)
{
	if (s_transmit_index < s_transmit_length)
	{
		UDR0 = s_tx_buffer_chars[s_transmit_index++];
	}
	else
	{
		disable_udre_interrupt();
	}
}

ISR(USART_TX_vect)
{
	if (s_transmit_index == s_transmit_length)
	{
		*sp_tx_complete_flag = true;
	}
}
