#include "generator.h"

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "mcu.h"

#include "dma.h"
#include "config.h"
#include "dac-samples.h"

#define GENERATOR_BUFFER_SIZE       (SAMPLE_BUFFER_SIZE)
#define IN_PHASE_PERIOD_COUNT       (2)

volatile SAMPLE_DATA_TYPE middlePointBuffer[GENERATOR_BUFFER_SIZE];
volatile SAMPLE_DATA_TYPE generatorBuffer[GENERATOR_BUFFER_SIZE];
uint8_t isPositivePhase = 1;
uint8_t periodCount = 0;

void initializeGenerator(void)
{
    isPositivePhase = 1;
	
	for (size_t i = 0; i < GENERATOR_BUFFER_SIZE; i++)
		middlePointBuffer[i] = middlePoint;
    
    disableDma(GENERATOR_DMA_STREAM);
    disableDma(GENERATOR_BUFFER_FILLER_DMA_STREAM);
    
    resetDmaFlags(GENERATOR_DMA_STREAM);
    resetDmaFlags(GENERATOR_BUFFER_FILLER_DMA_STREAM);
    
    GENERATOR_DMA_STREAM->PAR = (uint32_t)(&(GENERATOR_INTERFACE->ODR));
    GENERATOR_DMA_STREAM->M0AR = (uint32_t)positivePhase;
    GENERATOR_DMA_STREAM->M1AR = (uint32_t)positivePhase;
    GENERATOR_DMA_STREAM->NDTR = GENERATOR_BUFFER_SIZE;
    GENERATOR_DMA_STREAM->CR = GENERATOR_DMA_CONFIGURATION;
	GENERATOR_DMA_STREAM->FCR = DMA_SxFCR_DMDIS;
    
    enableDma(GENERATOR_DMA_STREAM);
    
    GENERATOR_BUFFER_FILLER_DMA_STREAM->CR = GENERATOR_BUFFER_FILLER_DMA_CONFIGURATION;
    
    GENERATOR_TIMER->CR1 = 0;
    GENERATOR_TIMER->PSC = 0;
    GENERATOR_TIMER->ARR = 5 - 1;
    GENERATOR_TIMER->CNT = 0;
    GENERATOR_TIMER->EGR = TIM_EGR_UG;
    GENERATOR_TIMER->SR = 0;
    GENERATOR_TIMER->DIER = TIM_DIER_UDE;
    GENERATOR_TIMER->CR1 = TIM_CR1_CEN;
}

void runGenerator(void)
{
    initializeGenerator();
    
    SAMPLE_DATA_TYPE *sampleBuffer = (isPositivePhase) ? positivePhase : negativePhase;
	volatile SAMPLE_DATA_TYPE *generatorBufferLink;
	volatile uint32_t *targetRegister;
	uint8_t counter = 0;
    
    while (1)
    {
		sampleBuffer = (isPositivePhase) ? positivePhase : negativePhase;
		
        while ((GENERATOR_DMA_CONTROLLER->HISR & DMA_HISR_TCIF5) == 0)
            ;
		
		if (GENERATOR_DMA_STREAM->CR & DMA_SxCR_CT)
			targetRegister = &(GENERATOR_DMA_STREAM->M0AR);
		else
			targetRegister = &(GENERATOR_DMA_STREAM->M1AR);
		
		*targetRegister = (uint32_t)sampleBuffer;
		
		GENERATOR_DMA_CONTROLLER->HIFCR = (DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5);
		
		periodCount++;
		
		if (periodCount == IN_PHASE_PERIOD_COUNT)
		{
			isPositivePhase ^= 1;
			periodCount = 0;
		}
    }
}
