/* 
   clover.c
   aleph-bfin

   four filter resonant network
*/

//-- blackfin toolchain headers
#include "fract_math.h"

//-- aleph/common headers
#include "types.h"

//-- aleph/bfin-lib headers
#include "bfin_core.h"        // global variables
#include "cv.h"               // cv output driver
#include "gpio.h"             // gpio pin numbers

//-- dsp class headers
#include "filter_1p.h"        // simple 1-pole integrator
#include "module.h"           // global declarations for module data

//--- custom headers
#include "params.h"           // parameter lists and constants


// customized module data structure
// this will be located at the top of SDRAM (64MB)
// all other variables (code, stack, heap) are located in SRAM (64K)
typedef struct _cloverData {
  //... here is where you would put other large data structures.

  // for example, a multichannel delay module would need an audio buffer:
  //  volatile fract32 audioBuffer[NUM_BUFS][FRAMES_PER_BUF];

  // bear in mind that access to SDRAM is significantly slower than SRAM (10-20 cycles!)
  // so don't put anything here that doesn't need the extra space.
} cloverData;

//-------------------------
//----- extern variables (initialized here)

// global pointer to module descriptor.
// required by the aleph-bfin library!
ModuleData* gModuleData;

//-----------------------bfin_lib/src/
//------ static variables

/* 
   here's the actual memory for module descriptor and param data
   global pointers are to point at these here during module init.
   we do it in this indirect way, because
   a) modules have variable param count
   b) in an extreme case, might need to locate param data in SDRAM
      ( until recently, SDRAM contained full param descriptors.)
*/
static ModuleData super;
static ParamData mParamData[eParamNumParams];

// input values
static fract32 adcVal[4];
static filter_1p_lo adcSlew[4];

// cv values (16 bits, but use fract32 and audio integrators)
static fract32 cutoffVal[4];
static filter_1p_lo cutoffSlew[4];
static fract32 cutoffVal[4];
static filter_1p_lo cutoffSlew[4];

// audio output bus
static fract32 outBus = 0;

//-----------------
//--- static function declaration

// small helper function to set parameter initial values
static inline void param_setup(u32 id, ParamValue v) {
  // set the input data so that bfin core will report it
  // back to the controller via SPI, when requested. 
  // (bees will make such a request of each param at launch to sync with network.)
  // this is also how a polled analysis parameter would work.
  gModuleData->paramData[id].value = v;
  module_set_param(id, v);
}


//----------------------
//----- external functions

void module_init(void) {
  // initialize module superclass data
  // by setting global pointers to our own data
  gModuleData = &super;
  gModuleData->paramData = mParamData;
  gModuleData->numParams = eParamNumParams;

  // initialize 1pole filters for input attenuation slew
  filter_1p_lo_init(&(adcSlew[0]), 0);
  filter_1p_lo_init(&(adcSlew[1]), 0);
  filter_1p_lo_init(&(adcSlew[2]), 0);
  filter_1p_lo_init(&(adcSlew[3]), 0);

  // Set initial param values
  // constants are from params.h

  //  filter 0
  // set amp to 1/4 (-12db) with right-shift intrinsic
  param_setup(eParam_adc0,      PARAM_AMP_MAX >> 2);
  param_setup(eParam_fb0,       0);
  param_setup(eParam_s1d0,      0);
  param_setup(eParam_s2d0,      0);
  param_setup(eParam_s3d0,      0);
  param_setup(eParam_fr0,       0);
  param_setup(eParam_rq0,       0);
  param_setup(eParam_mode0,     0);
  param_setup(eParam_gate0,     0);
  param_setup(eParam_slewGate0, PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewAdc0,  PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFb0,   PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFr0,   PARAM_SLEW_DEFAULT);

  //  filter 1
  param_setup(eParam_adc1,      0);
  param_setup(eParam_fb1,       0);
  param_setup(eParam_s0d1,      0);
  param_setup(eParam_s2d1,      0);
  param_setup(eParam_s3d1,      0);
  param_setup(eParam_fr1,       0);
  param_setup(eParam_rq1,       0);
  param_setup(eParam_mode1,     0);
  param_setup(eParam_gate1,     0);
  param_setup(eParam_slewGate1, PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewAdc1,  PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFb1,   PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFr1,   PARAM_SLEW_DEFAULT);

  //  filter 2
  param_setup(eParam_adc2,      0);
  param_setup(eParam_fb2,       0);
  param_setup(eParam_s0d2,      0);
  param_setup(eParam_s1d2,      0);
  param_setup(eParam_s3d2,      0);
  param_setup(eParam_fr2,       0);
  param_setup(eParam_rq2,       0);
  param_setup(eParam_mode2,     0);
  param_setup(eParam_gate2,     0);
  param_setup(eParam_slewGate2, PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewAdc2,  PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFb2,   PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFr2,   PARAM_SLEW_DEFAULT);

  //  filter 3
  param_setup(eParam_adc3,      0);
  param_setup(eParam_fb3,       0);
  param_setup(eParam_s0d3,      0);
  param_setup(eParam_s1d3,      0);
  param_setup(eParam_s2d3,      0);
  param_setup(eParam_fr3,       0);
  param_setup(eParam_rq3,       0);
  param_setup(eParam_mode3,     0);
  param_setup(eParam_gate3,     0);
  param_setup(eParam_slewGate3, PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewAdc3,  PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFb3,   PARAM_SLEW_DEFAULT);
  param_setup(eParam_slewFr3,   PARAM_SLEW_DEFAULT);
}

// de-init (never actually used on blackfin, but maybe by emulator)
void module_deinit(void) {
  ;;
}

// get number of parameters
u32 module_get_num_params(void) {
  return eParamNumParams;
}



// frame process function! 
// called each audio frame from codec interrupt handler
// ( bad, i know, see github issues list )
void module_process_frame(void) { 

  //--- process slew

  // 
  // update filters, calling "class method" on pointer to each
  adcVal[0] = filter_1p_lo_next(&(adcSlew[0]));
  adcVal[1] = filter_1p_lo_next(&(adcSlew[1]));
  adcVal[2] = filter_1p_lo_next(&(adcSlew[2]));
  adcVal[3] = filter_1p_lo_next(&(adcSlew[3]));

  //--- clover

  // zero the output bus
  outBus = 0;

  /* 
     note the use of fract32 arithmetic intrinsics!
     these are fast saturating multiplies/adds for 32bit signed fractions in [-1, 1)
     there are also intrinsics for fr16, mixed modes, and conversions.
     for details see:
     http://blackfin.uclinux.org/doku.php?id=toolchain:built-in_functions
  */
  // scale each input and add it to the bus
  /* outBus = add_fr1x32( outBus, mult_fr1x32x32(in[0], adcVal[0]) ); */
  /* outBus = add_fr1x32( outBus, mult_fr1x32x32(in[1], adcVal[1]) ); */
  /* outBus = add_fr1x32( outBus, mult_fr1x32x32(in[2], adcVal[2]) ); */
  /* outBus = add_fr1x32( outBus, mult_fr1x32x32(in[3], adcVal[3]) ); */

  // copy the bus to all the outputs
  out[0] = outBus;
  out[1] = outBus;
  out[2] = outBus;
  out[3] = outBus;
}


// parameter set function
void module_set_param(u32 idx, ParamValue v) {
  // switch on the param index
  switch(idx) {
    // input attenuation values
  case eParam_adc0:
    filter_1p_lo_in(&(adcSlew[0]), v);
    break;
  case eParam_adc1:
    filter_1p_lo_in(&(adcSlew[1]), v);
    break;
  case eParam_adc2:
    filter_1p_lo_in(&(adcSlew[2]), v);
    break;
  case eParam_adc3:
    filter_1p_lo_in(&(adcSlew[3]), v);
    break;

    // input attenuation slew values
  case eParam_slewAdc0:
   filter_1p_lo_set_slew(&(adcSlew[0]), v);
    break;
  case eParam_slewAdc1:
    filter_1p_lo_set_slew(&(adcSlew[1]), v);
    break;
  case eParam_slewAdc2:
    filter_1p_lo_set_slew(&(adcSlew[2]), v);
    break;
  case eParam_slewAdc3:
    filter_1p_lo_set_slew(&(adcSlew[3]), v);
    break;

  default:
    break;
  }
}
