#ifndef _DRIVER_TIMER_H_
#define _DRIVER_TIMER_H_

void Delay_us(int us);
void Delay_ms(int ms);
uint64_t GetSystemTimer_ns(void);

#endif