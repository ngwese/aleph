//#include <blackfin.h>
//#include <cdefBF533.h>  

//#include "bfin_core.h"
#include "gpio.h"
#include "cv.h"
#include "dma.h"

// AD5686R 16-bit quad DAC
#define CV_DAC_COM_WRITE 0x3
#define CV_DAC_COM_LSHIFT 20
#define CV_DAC_ADDR_LSHIFT 16

#define CV_DAC_VALUE_MASK 0xffff   // REMOVE, move to cv.c?

/// refactor; we need access to dma.c contents here

//! output buffers
cv_buffer_t cvOut = { 0, 0, 0, 0 };
cv_buffer_t cvTxBuf = { 0, 0, 0, 0 };

void init_cv(void) {
  u8 ch;
  u32 delay, buf;
  
  // bring the DAC out of reset
  CV_DAC_RESET_LO;
  delay = 100000;
  while(delay > 0) { delay--; }
  CV_DAC_RESET_HI;

  // setup output buffer commands and addresses
  for(ch=0; ch<CV_CHANNELS; ch++) {
    buf = 0;
    buf |= (CV_DAC_COM_WRITE << CV_DAC_COM_LSHIFT);
    buf |= ((1 << ch) << CV_DAC_ADDR_LSHIFT);
    cvTxBuf[ch] = buf;
  }
}

// initiate dac update
void cv_update(void) {
  // NB: testing irq DMA_RUN bit eliminates need for an isr but we
  // also have to wait for the dma tx fifo to be empty
  if (!dma_dac_running()) {
    // mux in write command and ch address
    cvTxBuf[0] |= (cvOut[0] >> 15) & CV_DAC_VALUE_MASK;
    cvTxBuf[1] |= (cvOut[1] >> 15) & CV_DAC_VALUE_MASK;
    cvTxBuf[2] |= (cvOut[2] >> 15) & CV_DAC_VALUE_MASK;
    cvTxBuf[3] |= (cvOut[3] >> 15) & CV_DAC_VALUE_MASK;
    
    // initiate DMA transfer
    dma_dac_update();
  }
}
