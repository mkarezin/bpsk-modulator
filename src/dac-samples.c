#include "dac-samples.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "config.h"

#define PI_NUMBER		(3.1415926535897932384626433832795)
#define DAC_BIT_WIDTH   (12)
#define MCU_DIGITAL_VOLTAGE     (3300)

typedef struct
{
	bool isPositivePhase;
	SAMPLE_DATA_TYPE *value;
} SampleBufferInfo;

SAMPLE_DATA_TYPE positivePhase[SAMPLE_PER_BIT] = { 0 };
SAMPLE_DATA_TYPE negativePhase[SAMPLE_PER_BIT] = { 0 };
SampleBufferInfo sampleBufferInfo[] =
{
	{
		.isPositivePhase = true,
		.value = positivePhase,
	},
	{
		.isPositivePhase = false,
		.value = negativePhase,
	},
};
uint32_t currentPeekToPeekVoltage = 0;
SAMPLE_DATA_TYPE middlePoint = 0;

static void calculateSample(SampleBufferInfo *sampleBufferInfo);
static SAMPLE_DATA_TYPE calculateADCSample(float calculatedSample);

void calculateSamples(uint32_t peekToPeekVoltage)
{
	currentPeekToPeekVoltage = peekToPeekVoltage;
	middlePoint = calculateADCSample((float)currentPeekToPeekVoltage / 2.0);
	
	for (size_t i = 0; i < (sizeof(sampleBufferInfo) / sizeof(*sampleBufferInfo)); i++)
	{
		memset(sampleBufferInfo[i].value, 0, (SAMPLE_PER_BIT * sizeof(SAMPLE_DATA_TYPE)));
		calculateSample(&sampleBufferInfo[i]);
	}
}

static void calculateSample(SampleBufferInfo *sampleBufferInfo)
{
	float startTime = 0.0;
	float endTime = (2 * PI_NUMBER);
	float signalPhase = (sampleBufferInfo->isPositivePhase) ? 0.0 : PI_NUMBER;
	float deltaTime = (endTime - startTime) / SAMPLE_PER_BIT;
	float currentTime = startTime;
	uint32_t amplitude = currentPeekToPeekVoltage >> 1;
	
	for (uint8_t i = 0; i < SAMPLE_PER_BIT; i++)
	{
		float calculatedSample = ((amplitude * sin(currentTime + signalPhase)) + amplitude);
		sampleBufferInfo->value[i] = calculateADCSample(calculatedSample);
		currentTime += deltaTime;
	}
}

static SAMPLE_DATA_TYPE calculateADCSample(float calculatedSample)
{
	return (SAMPLE_DATA_TYPE)( (calculatedSample * (1 << DAC_BIT_WIDTH)) / MCU_DIGITAL_VOLTAGE);
}
