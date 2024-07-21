#ifndef _DAC_SAMPLES_H_
#define _DAC_SAMPLES_H_

#include <stdint.h>

#include "config.h"

extern SAMPLE_DATA_TYPE middlePoint;
extern SAMPLE_DATA_TYPE positivePhase[];
extern SAMPLE_DATA_TYPE negativePhase[];

void calculateSamples(uint32_t peekToPeekVoltage);

#endif	//	_DAC_SAMPLES_H_
