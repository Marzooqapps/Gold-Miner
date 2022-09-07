



#ifndef __SYSTICK_H__ // do not include more than once
#define __SYSTICK_H__
#include <stdint.h>

void SysTick_Init(void);
void SysTick_Wait(uint32_t delay);
void SysTick_Wait100ms(uint32_t delay);


#endif // __TIMER2INTS_H__
