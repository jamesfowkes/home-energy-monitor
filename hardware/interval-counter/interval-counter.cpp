//#include <Wire.h>
#include <avr/interrupt.h>

#include <stdlib.h>

#include "check-and-clear.h"
#include "counters.h"

#include "lib_i2c_common.h"
#include "lib_i2c_config.h"

#include "ringbuf.h"
#include "interval.h"
#include "buffer.h"
#include "serial.h"
#include "time.h"
#include "rtc.h"

static const uint16_t SERIAL_BUFFER_SIZE = 16;

static char s_tx_serial_buffer[SERIAL_BUFFER_SIZE];
static volatile bool s_tx_complete = false;

static char s_rx_serial_buffer[SERIAL_BUFFER_SIZE];
static volatile bool s_rx_flag = false;

static bool s_ms_tick_flag;

static void send_next_interval()
{
	bool has_data = interval_fifo_pop(s_tx_serial_buffer);

	if (has_data)
	{
		serial_write(s_tx_serial_buffer);
	}
	else
	{
		serial_write("--\n");
	}
}

static void print_time()
{
	TIME t;
	time_get(t);
	time_print(t, s_tx_serial_buffer);
	serial_write(s_tx_serial_buffer);
}

static void print_count()
{
	serial_write(interval_fifo_count());
}

static void handle_query(char * rx_serial_buffer)
{
	switch(rx_serial_buffer[1])
	{
	case 'I':
		send_next_interval();
		break;
	case 'T':
		print_time();
		break;
	case 'C':
		print_count();
		break;
	}
}

static char get_command(char * rx_serial_buffer)
{
	return rx_serial_buffer[0];
}

static char get_setting(char * rx_serial_buffer)
{
	return rx_serial_buffer[1];
}

static void handle_timestamp(char * rx_serial_buffer)
{
	TIME timestamp;
	char * p_timestamp = &rx_serial_buffer[2];
	timestamp.h = (p_timestamp[0]-'0')*10 + (p_timestamp[1]-'0');
	timestamp.m = (p_timestamp[3]-'0')*10 + (p_timestamp[4]-'0');
	timestamp.s = (p_timestamp[6]-'0')*10 + (p_timestamp[7]-'0');
	timestamp.ms = (p_timestamp[9]-'0')*100 + (p_timestamp[10]-'0')*10 + (p_timestamp[11]-'0');
	time_set(timestamp);
}

static bool handle_setting(char * rx_serial_buffer)
{
	bool handled = true;
	switch (get_setting(rx_serial_buffer))
	{
	case 'T':
		handle_timestamp(rx_serial_buffer);
		break;
	case 'F':
		interval_trigger_pulse();
		break;
	default:
		handled = false;
		break;
	}
	return handled;
}

static void handle_command(char * rx_serial_buffer)
{
	switch (get_command(rx_serial_buffer))
	{
	case '?':
		handle_query(rx_serial_buffer);
		break;
	case '!':
		if (handle_setting(rx_serial_buffer))
		{
			serial_write("OK\n");
		}
		else
		{
			serial_write("NOK\n");
		}
		break;
	default:
		break;
	}
}

static bool command_is_pending(SerialBuffer& rx_serial_buffer)
{
	return rx_serial_buffer.last() == '\n';
}

static uint16_t s_heartbeat_timer = 500;

static void heartbeat_ms_tick()
{
	if (decrement_reset_at_zero(s_heartbeat_timer, 500U))
	{
		PINB |= (1<<5);
	}
}

static void setup_io()
{
	PORTD |= (1 << 2);
	DDRB |= (1 << 5);

	I2C_SetPrescaler(64);
	I2C_Init(NULL);
}

static void external_interrupt_enable()
{
	EIMSK = (1 << INT1) | (1 << INT0);
	EICRA = (1 << ISC11);
}

int main()
{
	setup_io();

	serial_setup(s_rx_serial_buffer, SERIAL_BUFFER_SIZE, (bool&)s_rx_flag, (bool&)s_tx_complete);

	sei();
	
	time_setup(s_ms_tick_flag);
	rtc_setup();

	serial_enable_interrupts(true);
	external_interrupt_enable();
	time_enable_interrupts();

	serial_write("RDY\n");

	while(true)
	{
		if (check_and_clear((bool&)s_rx_flag))
		{
			if (command_is_pending(serial_buffer()))
			{
				handle_command(s_rx_serial_buffer);
				serial_buffer().reset();
			}
		}

		interval_tick();

		if (check_and_clear(s_ms_tick_flag))
		{
			heartbeat_ms_tick();
		}
	}
}
