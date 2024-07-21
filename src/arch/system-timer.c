#include "system-timer.h"

#include <stdint.h>
#include <stdbool.h>

#include "mcu.h"
#include "system_stm32f4xx.h"

#include "config.h"

volatile uint32_t systemTimerTick = 0;
volatile Timer systemTimer = { 0 };
volatile uint32_t previousTime = 0;
volatile uint32_t timeout = 0;

void SysTick_Handler(void)
{
	systemTimerTick++;
	
	if (systemTimer.flag.isSet)
	{
		uint32_t currentTime = systemTimerTick;
		
		systemTimer.flag.alarm = ((currentTime - previousTime) >= timeout) ? true : false;
	
		if (systemTimer.flag.alarm)
		{
			systemTimer.flag.isSet = false;
			NVIC_DisableIRQ(SysTick_IRQn);
		}
		
		if (systemTimer.flag.delayedFlush && systemTimer.flag.alarm)
		{
			systemTimer.flag.delayedFlush = false;
			systemTimer.flag.alarm = false;
		}
	}
}

void updateSystemTimer(void)
{
	SystemCoreClockUpdate();
	return;
	uint32_t timebase = SystemCoreClock / SYSTEM_TIMER_TIMEBASE_FREQUENCY_HZ;
	SysTick_Config(timebase);
}

void setSystemTimer(uint32_t timeoutValue)
{
	return;
	systemTimer.flag.value = 0;
	
	if (timeoutValue == 0)
	{
		systemTimer.flag.alarm = true;
		return;
	}
	
	timeout = timeoutValue;
	systemTimerTick = 0;
	previousTime = systemTimerTick;
	
	systemTimer.flag.isSet = true;
	
	NVIC_EnableIRQ(SysTick_IRQn);
}

void flushSystemTimerAlarm(void)
{
	if (systemTimer.flag.isSet)
		systemTimer.flag.delayedFlush = true;
	else
	{
		systemTimer.flag.alarm = false;
		systemTimer.flag.delayedFlush = false;
	}
}
