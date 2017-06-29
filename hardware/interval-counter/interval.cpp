#include <avr/interrupt.h>

#include <stdio.h>
#include <string.h>

#include "ringbuf.h"
#include "time.h"

static char s_intervals[16][16];

static RingBuffer<char[16]> s_interval_buffer(s_intervals, 16, true);

static TIME s_last_pulse_time;
static bool s_pending_timestamp = false;

void interval_init()
{
}

bool interval_fifo_pop(char * buf)
{
	char interval[16];
	if (s_interval_buffer.pop_front(interval)) { return false; }

	strncpy(buf, interval, 16);

	return true;
}

uint8_t interval_fifo_count()
{
	return s_interval_buffer.count();
}

void interval_tick()
{
	char interval[16];
	if (s_pending_timestamp)
	{
		time_print(s_last_pulse_time, interval);

		s_interval_buffer.push_back(interval);

		s_pending_timestamp = false;
	}
}

void interval_trigger_pulse()
{
	cli();
	if (!s_pending_timestamp)
	{
		time_get(s_last_pulse_time);
		s_pending_timestamp = true;
	}
	sei();
}

ISR(INT0_vect)
{
	if (!s_pending_timestamp)
	{
		time_get(s_last_pulse_time);
		s_pending_timestamp = true;
	}
}
