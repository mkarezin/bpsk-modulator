#include "dma.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mcu.h"

#include "config.h"

static inline volatile uint32_t *getClearFlagRegister(DMA_Stream_TypeDef *dmaStream)
{
	if ((dmaStream == DMA1_Stream0) || (dmaStream == DMA1_Stream1) ||
		(dmaStream == DMA1_Stream2) || (dmaStream == DMA1_Stream3))
	{
		return &(DMA1->LIFCR);
	}
	else if ((dmaStream == DMA1_Stream4) || (dmaStream == DMA1_Stream5) ||
			 (dmaStream == DMA1_Stream6) || (dmaStream == DMA1_Stream7))
	{
		return &(DMA1->HIFCR);
	}
	else if ((dmaStream == DMA2_Stream0) || (dmaStream == DMA2_Stream1) ||
			 (dmaStream == DMA2_Stream2) || (dmaStream == DMA2_Stream3))
	{
		return &(DMA2->LIFCR);
	}
	else if ((dmaStream == DMA2_Stream4) || (dmaStream == DMA2_Stream5) ||
			 (dmaStream == DMA2_Stream6) || (dmaStream == DMA2_Stream7))
	{
		return &(DMA2->HIFCR);
	}
	
	return NULL;
}

static inline uint32_t getClearFlagMask(DMA_Stream_TypeDef *dmaStream)
{
	if ((dmaStream == DMA1_Stream0) || (dmaStream == DMA2_Stream0) ||
		(dmaStream == DMA1_Stream4) || (dmaStream == DMA2_Stream4))
	{
		return 0x3D;
	}
	else if ((dmaStream == DMA1_Stream1) || (dmaStream == DMA2_Stream1) ||
			 (dmaStream == DMA1_Stream5) || (dmaStream == DMA2_Stream5))
	{
		return 0xF40;
	}
	else if ((dmaStream == DMA1_Stream2) || (dmaStream == DMA2_Stream2) ||
			 (dmaStream == DMA1_Stream6) || (dmaStream == DMA2_Stream6))
	{
		return 0x3D0000;
	}
	
	/*
		((dmaStream == DMA1_Stream3) || (dmaStream == DMA2_Stream3) ||
		 (dmaStream == DMA1_Stream7) || (dmaStream == DMA2_Stream7))
	*/
	return 0xF400000;
}

bool dmaEnabled(DMA_Stream_TypeDef *dmaStream)
{
	return (dmaStream->CR & DMA_SxCR_EN);
}

void disableDma(DMA_Stream_TypeDef *dmaStream)
{
	dmaStream->CR &= ~DMA_SxCR_EN;
	while (dmaStream->CR & DMA_SxCR_EN)
		;
}

void enableDma(DMA_Stream_TypeDef *dmaStream)
{
	dmaStream->CR |= DMA_SxCR_EN;
	while ((dmaStream->CR & DMA_SxCR_EN) != DMA_SxCR_EN)
		;
}

void resetDmaFlags(DMA_Stream_TypeDef *dmaStream)
{
	volatile uint32_t *clearFlagRegister = getClearFlagRegister(dmaStream);
	uint32_t clearFlagMask = getClearFlagMask(dmaStream);
	
	if (clearFlagRegister == NULL)
		return;
	
	*clearFlagRegister = clearFlagMask;
}
