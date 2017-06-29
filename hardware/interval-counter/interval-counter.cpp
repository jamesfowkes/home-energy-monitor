//#include <Wire.h>
#include <avr/interrupt.h>

#include <stdlib.h>

#include "check-and-clear.h"

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

//static SerialBuffer s_rx_buffer(s_rx_chars, 0, 16);

static void send_next_interval()
{
	bool has_data = interval_fifo_pop(s_tx_serial_buffer);

	if (has_data)
	{
		serial_write(s_tx_serial_buffer);
	}
	else
	{
		serial_write("--");
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
			serial_write("OK");
		}
		else
		{
			serial_write("NOK");
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

//static bool s_heartbeat_led = false;
//static void heartbeat_task_fn(TaskAction * task)
//{
//	digitalWrite(13, s_heartbeat_led = !s_heartbeat_led);
//}
//static TaskAction s_heartbeat_task(heartbeat_task_fn, 500, INFINITE_TICKS);

static void cancel_all_timers()
{
	OCR2B = 0;
	OCR2A = 0;
	TCNT2 = 0;
	TCCR2B = 0;
	TCCR2A = 0;

	OCR1BH = 0;
	OCR1BL = 0;
	OCR1AH = 0;
	OCR1AL = 0;
	ICR1H = 0;
	ICR1L = 0;
	TCNT1H = 0;
	TCNT1L = 0;

	TCCR1C = 0;
	TCCR1B = 0;
	TCCR1A = 0;

	OCR0B = 0;
	OCR0A = 0;
	TCNT0 = 0;
	TCCR0B = 0;
	TCCR0A = 0;
}

static void setup_io()
{
	PORTD |= (1 << 2);
	DDRB |= (1 << 5);
}

int main()
{
	setup_io();

	serial_setup(s_rx_serial_buffer, SERIAL_BUFFER_SIZE, (bool&)s_rx_flag, (bool&)s_tx_complete);

	cancel_all_timers();

	sei();
	
	interval_init();
	rtc_setup();

	serial_enable_interrupts(true);

	serial_write("RDY");

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
		time_tick();

		//s_heartbeat_task.tick();
	}
}

ISR(INT1_vect)
{

}
