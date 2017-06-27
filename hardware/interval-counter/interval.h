#ifndef _INTERVAL_H_
#define _INTERVAL_H_

void interval_init();
bool interval_fifo_pop(char * buf);
uint8_t interval_fifo_count();
void interval_on_pulse();
void interval_tick();

#endif
