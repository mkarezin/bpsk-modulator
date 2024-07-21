#include "initialization.h"

#include "mcu.h"

#include "config.h"
#include "rcc.h"
#include "gpio.h"

static void initializeGpio(void);
static void initializeDma(void);
static void initializeTimer(void);

void mcuInitialization(void)
{
	initializeClock();
	
	initializeGpio();
	initializeDma();
    initializeTimer();
}

static void initializeGpio(void)
{
	PinSettings pinSettings = { 0 };
	
	//	DAC for Transmitter
	pinSettings.port = GENERATOR_INTERFACE;
	pinSettings.pinNumber = (pinNumber0 | pinNumber1 | pinNumber2 |
							 pinNumber3 | pinNumber4 | pinNumber5 |
							 pinNumber6 | pinNumber7 | pinNumber8 |
							 pinNumber9 | pinNumber10 | pinNumber11);
	pinSettings.mode = outputPinMode;
	pinSettings.type = pushPullPinType;
	pinSettings.speed = mediumPinSpeed;
	pinSettings.pull = noPullsPinPull;
	initializePin(&pinSettings);
}

static void initializeDma(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

static void initializeTimer(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
}
