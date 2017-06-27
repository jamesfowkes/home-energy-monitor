#include <Arduino.h>

#include <stdio.h>
#include <string.h>

#include "ringbuf.h"
#include "time.h"

static char s_intervals[32*16];

static RING_BUFFER s_interval_buffer;

static TIME s_last_pulse_time;
static bool s_pending_timestamp = false;

void interval_init()
{
	Ringbuf_Init(&s_interval_buffer, (RINGBUF_DATA)s_intervals, 16, 32, true);
}

bool interval_fifo_pop(char * buf)
{
	char * interval = (char*)Ringbuf_Pop_Front(&s_interval_buffer);

	if (!interval) { return false; }

	strncpy(buf, interval, 16);

	return true;
}

uint8_t interval_fifo_count()
{
	return Ringbuf_Count(&s_interval_buffer);
}

void interval_tick()
{
	char interval[16];
	if (s_pending_timestamp)
	{
		time_print(s_last_pulse_time, interval);

		Ringbuf_Put(&s_interval_buffer, (RINGBUF_DATA)interval);
		s_pending_timestamp = false;
	}
}

void interval_on_pulse()
{
	if (!s_pending_timestamp)
	{
		time_get(s_last_pulse_time);
		s_pending_timestamp = true;
	}
}
