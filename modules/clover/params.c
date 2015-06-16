/*

  params.c
  
  defines the module parameter descriptor fields.
   
  this is done in the descriptor helper program, 
  *not* in the module code itself!

  */

#include <string.h>
#include "module.h"
#include "params.h" 

extern void fill_param_desc(ParamDesc* desc) {
  //
  // filter 0; inputs
  //
  strcpy(desc[eParam_adc0].label, "adc0");
  desc[eParam_adc0].type = eParamTypeAmp; 
  desc[eParam_adc0].min = 0x00000000; 
  desc[eParam_adc0].max = PARAM_AMP_MAX;
  desc[eParam_adc0].radix = 16; // doesn't matter

  strcpy(desc[eParam_s1d0].label, "s1d0");
  desc[eParam_s1d0].type = eParamTypeAmp; 
  desc[eParam_s1d0].min = 0x00000000; 
  desc[eParam_s1d0].max = PARAM_AMP_MAX;
  desc[eParam_s1d0].radix = 16; // doesn't matter

  strcpy(desc[eParam_s2d0].label, "s2d0");
  desc[eParam_s2d0].type = eParamTypeAmp; 
  desc[eParam_s2d0].min = 0x00000000; 
  desc[eParam_s2d0].max = PARAM_AMP_MAX;
  desc[eParam_s2d0].radix = 16; // doesn't matter

  strcpy(desc[eParam_s3d0].label, "s3d0");
  desc[eParam_s3d0].type = eParamTypeAmp; 
  desc[eParam_s3d0].min = 0x00000000; 
  desc[eParam_s3d0].max = PARAM_AMP_MAX;
  desc[eParam_s3d0].radix = 16; // doesn't matter

  strcpy(desc[eParam_fb0].label, "fb0");
  desc[eParam_fb0].type = eParamTypeAmp; 
  desc[eParam_fb0].min = 0x00000000;      // FIXME: this should be bipolar for resonance
  desc[eParam_fb0].max = PARAM_AMP_MAX;
  desc[eParam_fb0].radix = 16; // doesn't matter

  // filter0; controls
  strcpy(desc[eParam_fr0].label, "cut0");
  desc[eParam_fr0].type = eParamTypeSvfFreq; 
  desc[eParam_fr0].min = 0x00000000; 
  desc[eParam_fr0].max = PARAM_CUT_MAX;
  desc[eParam_fr0].radix = 32;

  strcpy(desc[eParam_rq0].label, "rq0");
  desc[eParam_rq0].type = eParamTypeFix; 
  desc[eParam_rq0].min = PARAM_RQ_MIN;
  desc[eParam_rq0].max = PARAM_RQ_MAX;
  desc[eParam_rq0].radix = 2;

  strcpy(desc[eParam_mode0].label, "mode0"); // FIXME: pick a better type?
  desc[eParam_mode0].type = eParamTypeFix; 
  desc[eParam_mode0].min = 0x00000000; 
  desc[eParam_mode0].max = 0x0fffffff;
  desc[eParam_mode0].radix = 16;

  strcpy(desc[eParam_gate0].label, "gate0");
  desc[eParam_gate0].type = eParamTypeAmp; 
  desc[eParam_gate0].min = 0x00000000; 
  desc[eParam_gate0].max = PARAM_AMP_MAX;
  desc[eParam_gate0].radix = 16;

  // filter0; slewing
  strcpy(desc[eParam_slewGate0].label, "slewGate0");
  desc[eParam_slewGate0].type = eParamTypeIntegrator; 
  desc[eParam_slewGate0].min = 0x00000000; 
  desc[eParam_slewGate0].max = PARAM_SLEW_MAX;
  desc[eParam_slewGate0].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewAdc0].label, "slewAdc0");
  desc[eParam_slewAdc0].type = eParamTypeIntegrator; 
  desc[eParam_slewAdc0].min = 0x00000000; 
  desc[eParam_slewAdc0].max = PARAM_SLEW_MAX;
  desc[eParam_slewAdc0].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFb0].label, "slewFb0");
  desc[eParam_slewFb0].type = eParamTypeIntegrator; 
  desc[eParam_slewFb0].min = 0x00000000; 
  desc[eParam_slewFb0].max = PARAM_SLEW_MAX;
  desc[eParam_slewFb0].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFr0].label, "slewFr0");
  desc[eParam_slewFr0].type = eParamTypeIntegrator; 
  desc[eParam_slewFr0].min = 0x00000000; 
  desc[eParam_slewFr0].max = PARAM_SLEW_MAX;
  desc[eParam_slewFr0].radix = 16; // doesn't matter

  //
  // filter 1; inputs
  //
  strcpy(desc[eParam_adc1].label, "adc1");
  desc[eParam_adc1].type = eParamTypeAmp; 
  desc[eParam_adc1].min = 0x00000000; 
  desc[eParam_adc1].max = PARAM_AMP_MAX;
  desc[eParam_adc1].radix = 16; // doesn't matter

  strcpy(desc[eParam_s0d1].label, "s0d1");
  desc[eParam_s0d1].type = eParamTypeAmp; 
  desc[eParam_s0d1].min = 0x00000000; 
  desc[eParam_s0d1].max = PARAM_AMP_MAX;
  desc[eParam_s0d1].radix = 16; // doesn't matter

  strcpy(desc[eParam_s2d1].label, "s2d1");
  desc[eParam_s2d1].type = eParamTypeAmp; 
  desc[eParam_s2d1].min = 0x00000000; 
  desc[eParam_s2d1].max = PARAM_AMP_MAX;
  desc[eParam_s2d1].radix = 16; // doesn't matter

  strcpy(desc[eParam_s3d1].label, "s3d1");
  desc[eParam_s3d1].type = eParamTypeAmp; 
  desc[eParam_s3d1].min = 0x00000000; 
  desc[eParam_s3d1].max = PARAM_AMP_MAX;
  desc[eParam_s3d1].radix = 16; // doesn't matter

  strcpy(desc[eParam_fb1].label, "fb1");
  desc[eParam_fb1].type = eParamTypeAmp; 
  desc[eParam_fb1].min = 0x00000000;      // FIXME: this should be bipolar for resonance
  desc[eParam_fb1].max = PARAM_AMP_MAX;
  desc[eParam_fb1].radix = 16; // doesn't matter

  // filter 1; controls
  strcpy(desc[eParam_fr1].label, "cut1");
  desc[eParam_fr1].type = eParamTypeSvfFreq; 
  desc[eParam_fr1].min = 0x00000000; 
  desc[eParam_fr1].max = PARAM_CUT_MAX;
  desc[eParam_fr1].radix = 32;

  strcpy(desc[eParam_rq1].label, "rq1");
  desc[eParam_rq1].type = eParamTypeFix; 
  desc[eParam_rq1].min = PARAM_RQ_MIN;
  desc[eParam_rq1].max = PARAM_RQ_MAX;
  desc[eParam_rq1].radix = 2;

  strcpy(desc[eParam_mode1].label, "mode1"); // FIXME: pick a better type?
  desc[eParam_mode1].type = eParamTypeFix; 
  desc[eParam_mode1].min = 0x00000000; 
  desc[eParam_mode1].max = 0x0fffffff;
  desc[eParam_mode1].radix = 16;

  strcpy(desc[eParam_gate1].label, "gate1");
  desc[eParam_gate1].type = eParamTypeAmp; 
  desc[eParam_gate1].min = 0x00000000; 
  desc[eParam_gate1].max = PARAM_AMP_MAX;
  desc[eParam_gate1].radix = 16;

  // filter 1; slewing
  strcpy(desc[eParam_slewGate1].label, "slewGate1");
  desc[eParam_slewGate1].type = eParamTypeIntegrator; 
  desc[eParam_slewGate1].min = 0x00000000; 
  desc[eParam_slewGate1].max = PARAM_SLEW_MAX;
  desc[eParam_slewGate1].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewAdc1].label, "slewAdc1");
  desc[eParam_slewAdc1].type = eParamTypeIntegrator; 
  desc[eParam_slewAdc1].min = 0x00000000; 
  desc[eParam_slewAdc1].max = PARAM_SLEW_MAX;
  desc[eParam_slewAdc1].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFb1].label, "slewFb1");
  desc[eParam_slewFb1].type = eParamTypeIntegrator; 
  desc[eParam_slewFb1].min = 0x00000000; 
  desc[eParam_slewFb1].max = PARAM_SLEW_MAX;
  desc[eParam_slewFb1].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFr1].label, "slewFr1");
  desc[eParam_slewFr1].type = eParamTypeIntegrator; 
  desc[eParam_slewFr1].min = 0x00000000; 
  desc[eParam_slewFr1].max = PARAM_SLEW_MAX;
  desc[eParam_slewFr1].radix = 16; // doesn't matter

  //
  // filter 2; inputs
  //
  strcpy(desc[eParam_adc2].label, "adc2");
  desc[eParam_adc2].type = eParamTypeAmp; 
  desc[eParam_adc2].min = 0x00000000; 
  desc[eParam_adc2].max = PARAM_AMP_MAX;
  desc[eParam_adc2].radix = 16; // doesn't matter

  strcpy(desc[eParam_s0d2].label, "s0d2");
  desc[eParam_s0d2].type = eParamTypeAmp; 
  desc[eParam_s0d2].min = 0x00000000; 
  desc[eParam_s0d2].max = PARAM_AMP_MAX;
  desc[eParam_s0d2].radix = 16; // doesn't matter

  strcpy(desc[eParam_s1d2].label, "s1d2");
  desc[eParam_s1d2].type = eParamTypeAmp; 
  desc[eParam_s1d2].min = 0x00000000; 
  desc[eParam_s1d2].max = PARAM_AMP_MAX;
  desc[eParam_s1d2].radix = 16; // doesn't matter

  strcpy(desc[eParam_s3d2].label, "s3d2");
  desc[eParam_s3d2].type = eParamTypeAmp; 
  desc[eParam_s3d2].min = 0x00000000; 
  desc[eParam_s3d2].max = PARAM_AMP_MAX;
  desc[eParam_s3d2].radix = 16; // doesn't matter

  strcpy(desc[eParam_fb2].label, "fb2");
  desc[eParam_fb2].type = eParamTypeAmp; 
  desc[eParam_fb2].min = 0x00000000;      // FIXME: this should be bipolar for resonance
  desc[eParam_fb2].max = PARAM_AMP_MAX;
  desc[eParam_fb2].radix = 16; // doesn't matter

  // filter 2; controls
  strcpy(desc[eParam_fr2].label, "cut2");
  desc[eParam_fr2].type = eParamTypeSvfFreq; 
  desc[eParam_fr2].min = 0x00000000; 
  desc[eParam_fr2].max = PARAM_CUT_MAX;
  desc[eParam_fr2].radix = 32;

  strcpy(desc[eParam_rq2].label, "rq2");
  desc[eParam_rq2].type = eParamTypeFix; 
  desc[eParam_rq2].min = PARAM_RQ_MIN;
  desc[eParam_rq2].max = PARAM_RQ_MAX;
  desc[eParam_rq2].radix = 2;

  strcpy(desc[eParam_mode2].label, "mode2"); // FIXME: pick a better type?
  desc[eParam_mode2].type = eParamTypeFix; 
  desc[eParam_mode2].min = 0x00000000; 
  desc[eParam_mode2].max = 0x0fffffff;
  desc[eParam_mode2].radix = 16;

  strcpy(desc[eParam_gate2].label, "gate2");
  desc[eParam_gate2].type = eParamTypeAmp; 
  desc[eParam_gate2].min = 0x00000000; 
  desc[eParam_gate2].max = PARAM_AMP_MAX;
  desc[eParam_gate2].radix = 16;

  // filter 2; slewing
  strcpy(desc[eParam_slewGate2].label, "slewGate2");
  desc[eParam_slewGate2].type = eParamTypeIntegrator; 
  desc[eParam_slewGate2].min = 0x00000000; 
  desc[eParam_slewGate2].max = PARAM_SLEW_MAX;
  desc[eParam_slewGate2].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewAdc2].label, "slewAdc2");
  desc[eParam_slewAdc2].type = eParamTypeIntegrator; 
  desc[eParam_slewAdc2].min = 0x00000000; 
  desc[eParam_slewAdc2].max = PARAM_SLEW_MAX;
  desc[eParam_slewAdc2].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFb2].label, "slewFb2");
  desc[eParam_slewFb2].type = eParamTypeIntegrator; 
  desc[eParam_slewFb2].min = 0x00000000; 
  desc[eParam_slewFb2].max = PARAM_SLEW_MAX;
  desc[eParam_slewFb2].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFr2].label, "slewFr2");
  desc[eParam_slewFr2].type = eParamTypeIntegrator; 
  desc[eParam_slewFr2].min = 0x00000000; 
  desc[eParam_slewFr2].max = PARAM_SLEW_MAX;
  desc[eParam_slewFr2].radix = 16; // doesn't matter

  //
  // filter 3; inputs
  //
  strcpy(desc[eParam_adc3].label, "adc3");
  desc[eParam_adc3].type = eParamTypeAmp; 
  desc[eParam_adc3].min = 0x00000000; 
  desc[eParam_adc3].max = PARAM_AMP_MAX;
  desc[eParam_adc3].radix = 16; // doesn't matter

  strcpy(desc[eParam_s0d3].label, "s0d3");
  desc[eParam_s0d3].type = eParamTypeAmp; 
  desc[eParam_s0d3].min = 0x00000000; 
  desc[eParam_s0d3].max = PARAM_AMP_MAX;
  desc[eParam_s0d3].radix = 16; // doesn't matter

  strcpy(desc[eParam_s1d3].label, "s1d3");
  desc[eParam_s1d3].type = eParamTypeAmp; 
  desc[eParam_s1d3].min = 0x00000000; 
  desc[eParam_s1d3].max = PARAM_AMP_MAX;
  desc[eParam_s1d3].radix = 16; // doesn't matter

  strcpy(desc[eParam_s2d3].label, "s2d3");
  desc[eParam_s2d3].type = eParamTypeAmp; 
  desc[eParam_s2d3].min = 0x00000000; 
  desc[eParam_s2d3].max = PARAM_AMP_MAX;
  desc[eParam_s2d3].radix = 16; // doesn't matter

  strcpy(desc[eParam_fb3].label, "fb3");
  desc[eParam_fb3].type = eParamTypeAmp; 
  desc[eParam_fb3].min = 0x00000000;      // FIXME: this should be bipolar for resonance
  desc[eParam_fb3].max = PARAM_AMP_MAX;
  desc[eParam_fb3].radix = 16; // doesn't matter

  // filter 3; controls
  strcpy(desc[eParam_fr3].label, "cut3");
  desc[eParam_fr3].type = eParamTypeSvfFreq; 
  desc[eParam_fr3].min = 0x00000000; 
  desc[eParam_fr3].max = PARAM_CUT_MAX;
  desc[eParam_fr3].radix = 32;

  strcpy(desc[eParam_rq3].label, "rq3");
  desc[eParam_rq3].type = eParamTypeFix; 
  desc[eParam_rq3].min = PARAM_RQ_MIN;
  desc[eParam_rq3].max = PARAM_RQ_MAX;
  desc[eParam_rq3].radix = 2;

  strcpy(desc[eParam_mode3].label, "mode3"); // FIXME: pick a better type?
  desc[eParam_mode3].type = eParamTypeFix; 
  desc[eParam_mode3].min = 0x00000000; 
  desc[eParam_mode3].max = 0x0fffffff;
  desc[eParam_mode3].radix = 16;

  strcpy(desc[eParam_gate3].label, "gate3");
  desc[eParam_gate3].type = eParamTypeAmp; 
  desc[eParam_gate3].min = 0x00000000; 
  desc[eParam_gate3].max = PARAM_AMP_MAX;
  desc[eParam_gate3].radix = 16;

  // filter 3; slewing
  strcpy(desc[eParam_slewGate3].label, "slewGate3");
  desc[eParam_slewGate3].type = eParamTypeIntegrator; 
  desc[eParam_slewGate3].min = 0x00000000; 
  desc[eParam_slewGate3].max = PARAM_SLEW_MAX;
  desc[eParam_slewGate3].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewAdc3].label, "slewAdc3");
  desc[eParam_slewAdc3].type = eParamTypeIntegrator; 
  desc[eParam_slewAdc3].min = 0x00000000; 
  desc[eParam_slewAdc3].max = PARAM_SLEW_MAX;
  desc[eParam_slewAdc3].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFb3].label, "slewFb3");
  desc[eParam_slewFb3].type = eParamTypeIntegrator; 
  desc[eParam_slewFb3].min = 0x00000000; 
  desc[eParam_slewFb3].max = PARAM_SLEW_MAX;
  desc[eParam_slewFb3].radix = 16; // doesn't matter

  strcpy(desc[eParam_slewFr3].label, "slewFr3");
  desc[eParam_slewFr3].type = eParamTypeIntegrator; 
  desc[eParam_slewFr3].min = 0x00000000; 
  desc[eParam_slewFr3].max = PARAM_SLEW_MAX;
  desc[eParam_slewFr3].radix = 16; // doesn't matter
}

// EOF
