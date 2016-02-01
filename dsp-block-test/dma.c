#include <blackfin.h>
#include <cdefBF533.h>  
#include "ccblkfn.h"

#include "audio.h"
#include "dma.h"

#if DMA_DEINTERLEAVE_PINGPONG
// use large descriptor mode to perform pingpong and deinterleave

typedef struct {
  void *next;        // NDPL, NDPH
  void *start;       // SAL, SAH
  /*
  short config;      // DMACFG
  short x_count;     // XCNT
  short x_mod;       // XMOD
  short y_count;     // YCNT
  short y_mod;       // YMOD
  */
} dma_desc_t;

// for deinterleaving, inner loop over number of channels
#define X_COUNT CHANNELS
// each inner-loop increment, jump forward to the same frame in the next channel
#define X_MOD (SAMPLESIZE * BLOCKSIZE)
// outer loop over number of frames in a block
#define Y_COUNT BLOCKSIZE
// each outer-loop increment, jump back to the next frame in the first channel
#define Y_MOD (((1 - CHANNELS) * BLOCKSIZE + 1) * SAMPLESIZE)

// large descriptor list, first 4 registers only (NDPL, NDPH, SAL, SAH)
#define DMA_FLOW_DESC 0x7400
// NB: need interrupt on both TX and RX to ensure correct process order 
#define DMA_CONFIG ( DMA_FLOW_DESC | WDSIZE_32 | DI_EN | DMA2D )

dma_desc_t descRx1 = { NULL, inputChannels1 };        //, DMA_CONFIG | WNR, X_COUNT, X_MOD, Y_COUNT, Y_MOD };
dma_desc_t descRx0 = { &descRx1, inputChannels0 };    //, DMA_CONFIG | WNR, X_COUNT, X_MOD, Y_COUNT, Y_MOD };

dma_desc_t descTx1 = { NULL, outputChannels1 };       //, DMA_CONFIG, X_COUNT, X_MOD, Y_COUNT, Y_MOD };
dma_desc_t descTx0 = { &descTx1, outputChannels0 };   //, DMA_CONFIG, X_COUNT, X_MOD, Y_COUNT, Y_MOD };


void init_dma(void) {

  // map DMA1 to sport0 RX
  *pDMA1_PERIPHERAL_MAP = 0x1000;  
  // map DMA2 to sport0 TX
  *pDMA2_PERIPHERAL_MAP = 0x2000;
  
  // ping-pong
  descRx1.next = &descRx0;
  descTx1.next = &descTx0;

  *pDMA1_X_COUNT = X_COUNT;
  *pDMA1_X_MODIFY = X_MOD;
  *pDMA1_Y_COUNT = Y_COUNT;
  *pDMA1_Y_MODIFY = Y_MOD;
  *pDMA1_NEXT_DESC_PTR = &descRx0;
  //*pDMA1_START_ADDR = descRx0.start;
  *pDMA1_CONFIG = DMA_CONFIG | WNR;

  *pDMA2_X_COUNT = X_COUNT;
  *pDMA2_X_MODIFY = X_MOD;
  *pDMA2_Y_COUNT = Y_COUNT;
  *pDMA2_Y_MODIFY = Y_MOD;
  *pDMA2_NEXT_DESC_PTR = &descTx0;
  //*pDMA2_START_ADDR = descTx0.start;
  *pDMA2_CONFIG = DMA_CONFIG;
}

#else // 1d, autobuffer mode (no de-interleave, no pingpong

// DMA flow modes
 #define FLOW_AUTO 0x1000 

 void init_dma(void) {  
  
   // map DMA1 to Sport0 RX 
   *pDMA1_PERIPHERAL_MAP = 0x1000;   
   // map DMA2 to Sport0 TX 
   *pDMA2_PERIPHERAL_MAP = 0x2000; 

   // NB: need interrupt on both TX and RX to ensure correct process order 
   // dma1: rx 
   *pDMA1_CONFIG = WNR | WDSIZE_32 | DI_EN | FLOW_AUTO; 
   *pDMA1_START_ADDR = (void *)audioRxBuf; 
   *pDMA1_X_COUNT = CHANNELS * BLOCKSIZE; 
   *pDMA1_X_MODIFY = SAMPLESIZE; 
  
   // dma 2: tx 
   *pDMA2_CONFIG = WDSIZE_32 | DI_EN | FLOW_AUTO; 
   *pDMA2_START_ADDR = (void *)audioTxBuf; 
   *pDMA2_X_COUNT = CHANNELS * BLOCKSIZE; 
   *pDMA2_X_MODIFY = SAMPLESIZE; 

 } 

#endif
