#ifndef _TIME_H_
#define _TIME_H_

struct time
{
	uint8_t h;
	uint8_t m;
	uint8_t s;
	uint16_t ms;
};
typedef struct time TIME;

void time_set(TIME& time);
void time_get(TIME& time);
void time_tick();

void time_print(TIME& time, char * buf);

#endif
