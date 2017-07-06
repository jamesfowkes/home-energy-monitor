#include <Arduino.h>

#include "time.h"

static TIME s_time = {0,0,0,0};

static const uint16_t FTARGET = 1000;
static const uint16_t TOP = F_CPU / (uint32_t)FTARGET;

static bool* sp_ms_flag = NULL;

static __inline__ void time_add_h(TIME& time, uint16_t h)
{
	time.h += h;
	while (time.h > 23)
	{
		time.h -= 24;
	}
}

static __inline__ void time_add_m(TIME& time, uint16_t m)
{
	time.m += m;
	while (time.m > 59)
	{
		time.m -= 60;
		time_add_h(time, 1);
	}
}

static __inline__ void time_add_s(TIME& time, uint16_t s)
{
	time.s += s;
	while (time.s > 59)
	{
		time.s -= 60;
		time_add_m(time, 1);
	}
}

static __inline__ void time_add_ms(TIME& time, uint16_t ms)
{
	time.ms += ms;
	while (time.ms > 999U)
	{
		time.ms -= 1000U;
		time_add_s(time, 1);
	}
}
 
void time_setup(bool& ms_flag)
{
	sp_ms_flag = &ms_flag;
	OCR1A = TOP;
}

void time_enable_interrupts()
{
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);
}

void time_set(TIME& time)
{
	s_time.h = time.h;
	s_time.m = time.m;
	s_time.s = time.s;
	s_time.ms = time.ms;
}

void time_get(TIME& time)
{
	time.h = s_time.h;
	time.m = s_time.m;
	time.s = s_time.s;	
	time.ms = s_time.ms;
}

void time_print(TIME& time, char * buf)
{
	(void)time;
	sprintf(buf, "%02d:%02d:%02d.%03d\n", time.h, time.m, time.s, time.ms);
}

ISR(INT1_vect)
{
	TCNT1 = 0;
}

ISR(TIMER1_COMPA_vect)
{
	time_add_ms(s_time, 1);
	*sp_ms_flag = true;
}
