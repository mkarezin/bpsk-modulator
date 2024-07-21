#ifndef _DMA_H_
#define _DMA_H_

#include <stdbool.h>

#include "mcu.h"

bool dmaEnabled(DMA_Stream_TypeDef *dmaStream);
void disableDma(DMA_Stream_TypeDef *dmaStream);
void enableDma(DMA_Stream_TypeDef *dmaStream);
void resetDmaFlags(DMA_Stream_TypeDef *dmaStream);

#endif	//	_DMA_H_
