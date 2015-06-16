#ifndef _ALEPH_CLOVER_PARAMS_H_
#define _ALEPH_CLOVER_PARAMS_H_

#include "param_common.h"

// define some constants here for the param descriptor code
// here, all the parameter ranges are pretty simple.
#define PARAM_AMP_MAX 	0x7fffffff
#define PARAM_CV_MAX 	0x7fffffff
#define PARAM_SLEW_MAX 	0x7fffffff

// something pretty fast, but noticeable
#define PARAM_SLEW_DEFAULT 	0x7ffecccc

#define SMOOTH_FREQ_MIN 0x2000 // 1/8s
#define SMOOTH_FREQ_MAX 0x400000 // 64s
#define SMOOTH_FREQ_RADIX 7

// svf cutoff
#define PARAM_CUT_MAX     0x7fffffff
#define PARAM_CUT_DEFAULT 0x43D0A8EC // ~8k

// rq
#define PARAM_RQ_MIN 0x00000000
#define PARAM_RQ_MAX 0x0000ffff
#define PARAM_RQ_DEFAULT 0x0000FFF0


// enumerate parameters
// the order defined here must be matched in the descriptor !
enum params {
  //  filter 0
  eParam_adc0,               // adc multiplier
  eParam_fb0,                // feedback multiplier
  eParam_s1d0,               // filterN in multiplier
  eParam_s2d0,               // filterN in multiplier
  eParam_s3d0,               // filterN in multiplier
  eParam_fr0,                // cutoff frequency
  eParam_rq0,                // recipricol q
  eParam_mode0,              // mode dial lp -> bp -> hp
  eParam_gate0,              // ouput gate
  eParam_slewGate0,
  eParam_slewAdc0,
  eParam_slewFb0,
  eParam_slewFr0,

  //  filter 1
  eParam_adc1,               // adc multiplier
  eParam_fb1,                // feedback multiplier
  eParam_s0d1,               // filterN in multiplier
  eParam_s2d1,               // filterN in multiplier
  eParam_s3d1,               // filterN in multiplier
  eParam_fr1,                // cutoff frequency
  eParam_rq1,                // recipricol q
  eParam_mode1,              // mode dial lp -> bp -> hp
  eParam_gate1,              // ouput gate
  eParam_slewGate1,
  eParam_slewAdc1,
  eParam_slewFb1,
  eParam_slewFr1,

  //  filter 2
  eParam_adc2,               // adc multiplier
  eParam_fb2,                // feedback multiplier
  eParam_s0d2,               // filterN in multiplier
  eParam_s1d2,               // filterN in multiplier
  eParam_s3d2,               // filterN in multiplier
  eParam_fr2,                // cutoff frequency
  eParam_rq2,                // recipricol q
  eParam_mode2,              // mode dial lp -> bp -> hp
  eParam_gate2,              // ouput gate
  eParam_slewGate2,
  eParam_slewAdc2,
  eParam_slewFb2,
  eParam_slewFr2,

  //  filter 3
  eParam_adc3,               // adc multiplier
  eParam_fb3,                // feedback multiplier
  eParam_s0d3,               // filterN in multiplier
  eParam_s1d3,               // filterN in multiplier
  eParam_s2d3,               // filterN in multiplier
  eParam_fr3,                // cutoff frequency
  eParam_rq3,                // recipricol q
  eParam_mode3,              // mode dial lp -> bp -> hp
  eParam_gate3,              // ouput gate
  eParam_slewGate3,
  eParam_slewAdc3,
  eParam_slewFb3,
  eParam_slewFr3,

  eParamNumParams
};

// this is only defined by the descriptor helper program.
extern void fill_param_desc(ParamDesc* desc);

#endif // header guard 
// EOF
