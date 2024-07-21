#ifndef _SYSTEM_TIMER_H_
#define _SYSTEM_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

#include "mcu.h"

typedef struct
{
	union
	{
		uint8_t value;
		struct
		{
			bool isSet : 1;
			bool alarm : 1;
			bool delayedFlush : 1;
			uint8_t reserved : 5;
		};
	} flag;
} Timer;

extern volatile uint32_t systemTimerTick;
extern volatile Timer systemTimer;

void SysTick_Handler(void);
void updateSystemTimer(void);
void setSystemTimer(uint32_t timeoutValue);
void flushSystemTimerAlarm(void);

#endif	//	_SYSTEM_TIMER_H_
