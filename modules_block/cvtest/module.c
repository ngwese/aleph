#include <string.h>

#include "module.h"
#include "params.h"

ModuleData* gModuleData;
static ModuleData super;
static ParamData mParamData[eParamNumParams];

static fract32 cv;

static inline void param_setup(u32 id, ParamValue v) {
  gModuleData->paramData[id].value = v;
  module_set_param(id, v);
}

void module_init(void) {
  //u16 i;

  gModuleData = &super;
  strcpy(gModuleData->name, "cvteset");
  gModuleData->paramData = mParamData;
  gModuleData->numParams = eParamNumParams;

  cv = 0;
}

void module_process_block(buffer_t *inChannels, buffer_t *outChannels) { 
  u16 i, j;

  // audio pass through
  for(i=0; i<AUDIO_CHANNELS; i++) {
    for(j=0; j<MODULE_BLOCKSIZE; j++) {
      (*outChannels)[i][j] = (*inChannels)[i][j];
    }
  }

  // cv output
  for(i=0; i<CV_CHANNELS; i++) {
    cvOut[i] = cv;
  }

  // value for next cycle
  cv = (cv + 1) % CV_MAX_VALUE;
  
  cv_update();
}

void module_set_param(u32 idx, ParamValue val) {
    /* switch(idx) { */

    /* case eParamAmp0: */
    /*   osc_set_amp(0, val ); */
    /*   break; */

    /* case eParamFreqCoarse0: */
    /*   osc_set_phi_upper(0, val ); */
    /*   break; */

    /* case eParamFreqFine0: */
    /*   osc_set_phi_lower(0, val ); */
    /*   break; */
      
    /* case eParamAmp1: */
    /*   osc_set_amp(1, val ); */
    /*   break; */

    /* case eParamFreqCoarse1: */
    /*   osc_set_phi_upper(1, val ); */
    /*   break; */

    /* case eParamFreqFine1: */
    /*   osc_set_phi_lower(1, val ); */
    /*   break; */
      
    /* case eParamAmp2: */
    /*   osc_set_amp(2, val ); */
    /*   break; */

    /* case eParamFreqCoarse2: */
    /*   osc_set_phi_upper(2, val ); */
    /*   break; */

    /* case eParamFreqFine2: */
    /*   osc_set_phi_lower(2, val ); */
    /*   break; */

    /* case eParamAmp3: */
    /*   osc_set_amp(3, val ); */
    /*   break; */

    /* case eParamFreqCoarse3: */
    /*   osc_set_phi_upper(3, val ); */
    /*   break; */

    /* case eParamFreqFine3: */
    /*   osc_set_phi_lower(3, val ); */
    /*   break; */

    /* } */
}
