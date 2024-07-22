#include "rcc.h"

#include <stdint.h>

#include "mcu.h"

#include "system-timer.h"
#include "config.h"
#include "gpio.h"

#define PLLM_MIN_FREQUENCY		(950000)
#define PLLM_MAX_FREQUENCY		(2100000)
#define PLLN_MIN_FREQUENCY		(100000000)
#define PLLN_MAX_FREQUENCY		(432000000)
#define PLLP_MIN_FREQUENCY		(24000000)
#define PLLP_MAX_FREQUENCY		(100000000)
#define PLLQ_MIN_FREQUENCY		(0)
#define PLLQ_MAX_FREQUENCY		(48000000)

#if !defined (HSE_VALUE)
	#define HSE_VALUE		(8000000)
#endif	//	HSE_VALUE
#if !defined (HSI_VALUE)
	#define HSI_VALUE		(16000000)
#endif	//	HSI_VALUE

typedef enum
{
	hsi = 1,
	hse,
	pll,
} ClockSource;

typedef struct
{
	ClockSource clockSource;
	uint8_t pllM;
	uint16_t pllN;
	uint8_t pllP;
	uint8_t pllQ;
} PllSettings;

typedef enum
{
	AhbDividedBy1 = 0,
	AhbDividedBy2 = 0x08,
	AhbDividedBy4,
	AhbDividedBy8,
	AhbDividedBy16,
	AhbDividedBy64,
	AhbDividedBy128,
	AhbDividedBy256,
	AhbDividedBy512,
} AhbPrescaler;

typedef enum
{
	ApbDividedBy1 = 0,
	ApbDividedBy2 = 0x04,
	ApbDividedBy4,
	ApbDividedBy8,
	ApbDividedBy16,
} ApbPrescaler;

typedef enum
{
	McoDividedBy1 = 0,
	McoDividedBy2 = 0x04,
	McoDividedBy3,
	McoDividedBy4,
	McoDividedBy5,
} McoPrescaler;

typedef enum
{
	McoSourceNone = -1,
	McoSourceSystemClock = 0,
	McoSourcePllI2SClock,
	McoSourceHseClock,
	McoSourcePllClock,
	McoSourceHsiClock,
	McoSourceLseClock,
} McoSource;

typedef struct
{
	McoSource clockSource;
	McoPrescaler prescaler;
} McoSettings;

typedef enum
{
	vosScale3 = 1,
	vosScale2,
	vosScale1,
} VosScale;

typedef struct
{
	uint32_t targetSystemClock;
	ClockSource systemClockSource;
	PllSettings pllSettings;
	AhbPrescaler ahbPrescaler;
	ApbPrescaler apb1Prescaler;
	ApbPrescaler apb2Prescaler;
	McoSettings mco1;
	McoSettings mco2;
	bool overdrive;
} ClockSettings;

ClockSettings clockSettings =
{
	.targetSystemClock = HCLK,
	.systemClockSource = pll,
	.pllSettings =
	{
		.clockSource = hsi,
		.pllM = 0,
		.pllN = 0,
		.pllP = 0,
		.pllQ = 0,
	},
	.ahbPrescaler = AhbDividedBy1,
	.apb1Prescaler = ApbDividedBy2,
	.apb2Prescaler = ApbDividedBy1,
	.mco1 = { .clockSource = McoSourceNone },
	#ifdef DEBUG_MCU_SYSCLK
	.mco2 = { .clockSource = McoSourceSystemClock, .prescaler = McoDividedBy5 },
	#else
	.mco2 = { .clockSource = McoSourceNone },
	#endif	//	DEBUG_MCU_SYSCLK
	.overdrive = true,
};
ClockSettings defaultClockSettings =
{
	.overdrive = false,
	.targetSystemClock = DEFAULT_SYSTEM_CLOCK_FREQUENCY,
	.systemClockSource = hsi,
	.pllSettings = { 0 },
	.ahbPrescaler = AhbDividedBy1,
	.apb1Prescaler = ApbDividedBy2,
	.apb2Prescaler = ApbDividedBy1,
	.mco1 = { .clockSource = McoSourceNone },
	.mco2 = { .clockSource = McoSourceNone },
};

static bool configureMco(ClockSettings *settings);
static bool enableClockSource(ClockSettings *settings);
static bool configurePll(ClockSettings *settings);
static bool calculatePllComponents(ClockSettings *settings);
static bool calculatePllNComponent(ClockSettings *settings, const uint32_t sourceFrequency);
static bool calculatePllPComponent(ClockSettings *settings, const uint32_t sourceFrequency);
static bool calculatePllQComponent(ClockSettings *settings, const uint32_t sourceFrequency);
static bool enableOverdrive(ClockSettings *settings);
static bool enablePll(void);

void initializeClock(void)
{
#ifdef DEBUG_MCU_SYSCLK
	PinSettings pinSettings = { 0 };
	
	pinSettings.port = GPIOC;
	pinSettings.pinNumber = pinNumber9;
	pinSettings.mode = alternateFunctionPinMode;
	pinSettings.type = pushPullPinType;
	pinSettings.speed = veryHighPinSpeed;
	pinSettings.pull = noPullsPinPull;
	pinSettings.alternateFunctionNumber = alternateFunction0;
	
	initializePin(&pinSettings);
#endif	//	DEBUG_MCU_SYSCLK
	do
	{
		updateSystemTimer();
		
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		
		if (!configureMco(&clockSettings))
			break;
		
		if (!enableClockSource(&clockSettings))
			break;
		
		if (!configurePll(&clockSettings))
			break;
		
		if (clockSettings.overdrive)
		{
			if (!enableOverdrive(&clockSettings))
				break;
		}
		
		if (!enablePll())
			break;
		
		FLASH->ACR |= FLASH_ACR_LATENCY_3WS;
		FLASH->ACR |= FLASH_ACR_PRFTEN;
		
		uint32_t temp = RCC->CFGR;
		temp &= ~(RCC_CFGR_PPRE2_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_HPRE_Msk | RCC_CFGR_SW_Msk);
		temp |= (RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1);
		temp |= RCC_CFGR_SW_PLL;
		RCC->CFGR = temp;
		while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL)	;
		
		//RCC->CR &= ~RCC_CR_HSION;
		
		updateSystemTimer();
		
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD;
		setSystemTimer(100000);
		while ((SYSCFG->CMPCR & SYSCFG_CMPCR_READY) != SYSCFG_CMPCR_READY)
		{
			if (systemTimer.flag.alarm)
			{
				flushSystemTimerAlarm();
				SYSCFG->CMPCR &= ~SYSCFG_CMPCR_CMP_PD;
				break;
			}
		}
		flushSystemTimerAlarm();
		
		return;
	} while (0);
}

static bool configureMco(ClockSettings *settings)
{
	bool success = true;
	uint32_t mcoClockSource = 0;
	uint32_t mcoPrescaler = 0;
	uint8_t bitCombination = 0;
	
	if (settings->mco1.clockSource != McoSourceNone)
	{
		bitCombination = 0;
		
		if (settings->mco1.clockSource == McoSourceHsiClock)
			bitCombination = 0;
		else if (settings->mco1.clockSource == McoSourceLseClock)
			bitCombination = 1;
		else if (settings->mco1.clockSource == McoSourceHseClock)
			bitCombination = 2;
		else if (settings->mco1.clockSource == McoSourcePllClock)
			bitCombination = 3;
		
		mcoClockSource |= (bitCombination << RCC_CFGR_MCO1_Pos);
		mcoPrescaler |= (settings->mco1.prescaler << RCC_CFGR_MCO1PRE_Pos);
	}
	if (settings->mco2.clockSource != McoSourceNone)
	{
		bitCombination = 0;
		
		if (settings->mco2.clockSource == McoSourceSystemClock)
			bitCombination = 0;
		else if (settings->mco2.clockSource == McoSourcePllI2SClock)
			bitCombination = 1;
		else if (settings->mco2.clockSource == McoSourceHseClock)
			bitCombination = 2;
		else if (settings->mco2.clockSource == McoSourcePllClock)
			bitCombination = 3;
		
		mcoClockSource |= (bitCombination << RCC_CFGR_MCO2_Pos);
		mcoPrescaler |= (settings->mco2.prescaler << RCC_CFGR_MCO2PRE_Pos);
	}
	
	uint32_t temp = RCC->CFGR;
	temp &= ~(RCC_CFGR_MCO1_Msk | RCC_CFGR_MCO2_Msk |
			  RCC_CFGR_MCO1PRE_Msk | RCC_CFGR_MCO2PRE_Msk);
	temp |= (mcoClockSource | mcoPrescaler);
	RCC->CFGR = temp;

	return success;
}

static bool enableClockSource(ClockSettings *settings)
{
	uint32_t enableClockSource = 0;
	uint32_t clockSourceReadyFlag = 0;
	uint32_t clockStartupTimeoutUs = 0;
	bool success = true;
	
	if ((settings->systemClockSource == hsi) ||
		(settings->pllSettings.clockSource == hsi))
	{
		enableClockSource = RCC_CR_HSION;
		clockSourceReadyFlag = RCC_CR_HSIRDY;
		clockStartupTimeoutUs = 5;
	}
	else if ((settings->systemClockSource == hse) ||
			 (settings->pllSettings.clockSource == hse))
	{
		enableClockSource = RCC_CR_HSEON;
		clockSourceReadyFlag = RCC_CR_HSERDY;
		clockStartupTimeoutUs = 3000;
	}
	else
		return false;
	
	RCC->CR |= enableClockSource;
	setSystemTimer(clockStartupTimeoutUs);
	while ((RCC->CR & clockSourceReadyFlag) != clockSourceReadyFlag)
	{
		if (systemTimer.flag.alarm)
		{
			RCC->CR &= ~enableClockSource;
			success = false;
			break;
		}
	}
	
	flushSystemTimerAlarm();
	return success;
}

static bool configurePll(ClockSettings *settings)
{
	if ((settings->pllSettings.clockSource != hsi) && (settings->pllSettings.clockSource != hse))
		return false;
	
	settings->pllSettings.pllM = 0;
	settings->pllSettings.pllN = 0;
	settings->pllSettings.pllP = 0;
	settings->pllSettings.pllQ = 0;
	
	if (!calculatePllComponents(settings))
		return false;
	
	if ( ((settings->pllSettings.pllQ < 2) || (settings->pllSettings.pllQ > 15)) ||
		 ( (settings->pllSettings.pllP != 2) && (settings->pllSettings.pllP != 4) &&
		   (settings->pllSettings.pllP != 6) && (settings->pllSettings.pllP != 8) ) ||
		 ((settings->pllSettings.pllN < 2) || (settings->pllSettings.pllN > 432)) ||
		 ((settings->pllSettings.pllM < 2) || (settings->pllSettings.pllM > 63)))
	{
		return false;
	}

	uint32_t pllSettings = RCC->PLLCFGR;
	
	pllSettings &= ~(RCC_PLLCFGR_PLLQ_Msk | RCC_PLLCFGR_PLLM_Msk |
					 RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLP_Msk |
					 RCC_PLLCFGR_PLLSRC_Msk);
	
	pllSettings |= (settings->pllSettings.pllM << RCC_PLLCFGR_PLLM_Pos);
	pllSettings |= (settings->pllSettings.pllN << RCC_PLLCFGR_PLLN_Pos);
	pllSettings |= (((settings->pllSettings.pllP - 1) / 2) << RCC_PLLCFGR_PLLP_Pos);
	pllSettings |= (settings->pllSettings.pllQ << RCC_PLLCFGR_PLLQ_Pos);
	
	if (settings->pllSettings.clockSource == hsi)
		pllSettings |= RCC_PLLCFGR_PLLSRC_HSI;
	else if (settings->pllSettings.clockSource == hse)
		pllSettings |= RCC_PLLCFGR_PLLSRC_HSE;
	
	RCC->PLLCFGR = pllSettings;
	
	return true;
}

static bool calculatePllComponents(ClockSettings *settings)
{
	uint8_t pllM = 2;
	uint32_t sourceFrequency = (settings->pllSettings.clockSource == hse) ? HSE_VALUE : HSI_VALUE;
	
	for (; pllM < 64; pllM++)
	{
		uint32_t frequency = sourceFrequency / pllM;
		
		if ((frequency >= PLLM_MIN_FREQUENCY) && (frequency <= PLLM_MAX_FREQUENCY))
		{
			settings->pllSettings.pllM = pllM;
			
			if (calculatePllNComponent(settings, frequency))
				return true;
		}
	}
	
	return false;
}

static bool calculatePllNComponent(ClockSettings *settings, const uint32_t sourceFrequency)
{
	uint16_t pllN = 50;
	
	for (; pllN < 433; pllN++)
	{
		uint32_t frequency = sourceFrequency * pllN;
		
		if ((frequency >= PLLN_MIN_FREQUENCY) && (frequency <= PLLN_MAX_FREQUENCY))
		{
			settings->pllSettings.pllN = pllN;
			
			if (calculatePllPComponent(settings, frequency) && calculatePllQComponent(settings, frequency))
				return true;
		}
	}
	
	return false;
}

static bool calculatePllPComponent(ClockSettings *settings, const uint32_t sourceFrequency)
{
	uint8_t pllP = 2;
	
	for (; pllP < 10; pllP += 2)
	{
		uint32_t frequency = sourceFrequency / pllP;
		
		if ((frequency >= PLLP_MIN_FREQUENCY) && (frequency <= PLLP_MAX_FREQUENCY) && (frequency == settings->targetSystemClock))
		{
			settings->pllSettings.pllP = pllP;
			return true;
		}
	}
	
	return false;
}

static bool calculatePllQComponent(ClockSettings *settings, const uint32_t sourceFrequency)
{
	uint8_t pllQ = 2;
	
	for (; pllQ < 16; pllQ++)
	{
		uint32_t frequency = sourceFrequency / pllQ;
		
		if ((frequency >= PLLQ_MIN_FREQUENCY) && (frequency <= PLLQ_MAX_FREQUENCY))
		{
			settings->pllSettings.pllQ = pllQ;
			return true;
		}
	}
		
	return false;
}

static bool enableOverdrive(ClockSettings *settings)
{
	if ((RCC->CFGR & RCC_CFGR_SW_HSE) != RCC_CFGR_SW_HSE &&
		((RCC->CFGR & RCC_CFGR_SW_HSI) != RCC_CFGR_SW_HSI))
	{
		return false;
	}
	
	bool success = true;
	
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
	VosScale scale = vosScale3;
	
	if (settings->systemClockSource == pll)
		scale = vosScale1;
	
	if ((RCC->CR & RCC_CR_PLLON) != RCC_CR_PLLON)
	{
		uint32_t temp = PWR->CR;
		temp &= ~(PWR_CR_VOS_Msk);
		temp |= ((uint32_t)scale << PWR_CR_VOS_Pos);
		PWR->CR = temp;
	}
	
	return success;
}

static bool enablePll(void)
{
	bool success = true;
	uint32_t pllStartupTimeout = 500;
	
	RCC->CR |= RCC_CR_PLLON;
	setSystemTimer(pllStartupTimeout);
	while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
	{
		if (systemTimer.flag.alarm)
		{
			RCC->CR &= ~RCC_CR_PLLON;
			success = false;
			break;
		}
	}
	
	flushSystemTimerAlarm();
	
	return success;
}
