#ifndef _INTERVAL_H_
#define _INTERVAL_H_

void interval_setup();
bool interval_fifo_pop(char * buf);
uint16_t interval_fifo_count();
void interval_trigger_pulse();
void interval_tick();

#endif
