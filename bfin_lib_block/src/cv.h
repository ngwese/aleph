#ifndef _BFIN_CV_H_
#define _BFIN_CV_H_

#include "types.h"

#define CV_CHANNELS 4
#define CV_SAMPLESIZE 4

//! type for cv dac buffer
typedef fract32 cv_buffer_t[CV_CHANNELS];

//! pending cv/dac value buffer
__attribute__((l1_data_B))
__attribute__((aligned(32)))
extern cv_buffer_t cvOut;

//! dma tx buffer
__attribute__((l1_data_B))
__attribute__((aligned(32)))
extern cv_buffer_t cvTxBuf;


// initialize cv dac
extern void init_cv(void);

// update all cv channels 
// input values are assumed to be in [0, 0x7fffffff]
extern void cv_update(void);

#endif // header guard
