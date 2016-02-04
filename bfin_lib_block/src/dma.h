#ifndef _BFIN_DMA_H_
#define _BFIN_DMA_H_

#include "types.h"

extern void init_dma(void);

extern void dma_dac_update(void);
extern short dma_dac_running(void);

#endif
