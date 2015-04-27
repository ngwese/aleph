// std
#include <string.h>
// asf
#include "print_funcs.h"
// aleph-avr32
#include "memory.h"
// bees
#include "files.h"
#include "param_scaler.h"
#include "scaler_amp.h"

// table size
static const u8 tabBits = 10;
static const u32 tabSize = 1024;
// shift from io_t size to index
static const u8 inRshift = 5;

static const s32* tabVal;
static const s32* tabRep;

static u8 initFlag = 0;

//-------------------
//--- static funcs

//-----------------------
//---- extern funcs

s32 scaler_amp_val(void* scaler, io_t in) {
  if(in < 0) { in = 0; }
  return tabVal[(u16)((u16)in >> inRshift)];
}

void scaler_amp_str(char* dst, void* scaler,  io_t in) {
  //  u16 uin = BIT_ABS_16((s16)in) >> inRshift;
  if(in < 0) { in = 0; }

  in >>= inRshift;

  if(in == 0) {
    strcpy(dst, "   -inf");
  } else if (in == (tabSize - 1)) {
    print_fix16(dst, 0);
  } else {
    print_fix16(dst, tabRep[(u16)in] );
  }
}

// init function
void scaler_amp_init(void* scaler) {
  ParamScaler* sc = (ParamScaler*)scaler;
  //print_dbg("\r\n initializing amp scaler for param, label: ");
  //print_dbg(sc->desc->label);
  // check descriptor
  if( sc->desc->type != eParamTypeAmp) {
    //print_dbg("\r\n !!! warning: wrong param type for amp scaler");
    //print_dbg(" ; this param has type: ");
    //print_dbg_ulong(sc->desc->type);
  }
  
  // init flag for static data
  if(initFlag) { 
    ;;
  } else {
    initFlag = 1;

    // assign
    tabVal = scaler_get_nv_data(eParamTypeAmp);
    tabRep = scaler_get_nv_rep(eParamTypeAmp);

  }

    sc->inMin = 0;
    sc->inMax = (tabSize - 1) << inRshift;
  /// FIXME: should consider requested param range,
  //  and compute a customized multiplier here if necessary.
 
}

// get input given DSP value (use sparingly)
io_t scaler_amp_in(void* scaler, s32 x) {
  // value table is monotonic, can binary search
  s32 jl = 0;
  s32 ju = tabSize - 1;
  s32 jm;

  /// FIXME: this search result is often off by one, or something like it.

  // first, cheat and check zero.
  /// will often be true
  if(x == 0) { return 0; }

  while(ju - jl > 1) {
    jm = (ju + jl) >> 1;
    // value table is always ascending
    if(x >= tabVal[jm]) {
      jl = jm;
    } else {
      ju = jm;
    }
  }
  return (u16)jm << inRshift;
}


// increment input by pointer, return value
s32 scaler_amp_inc(void* scaler, io_t* pin, io_t inc ) {
  ParamScaler* sc = (ParamScaler*)scaler;
  // this speeds up the knob a great deal.
#if 0
  s32 sinc;
  // scale up to smallest significant abscissa:
  // check for 16b overflow
  sinc = (s32)inc << inRshift;
  if(sinc > 0x3fff) { 
    inc = (io_t)0x3fff;
  } 
  else if (sinc < (s32)0xffffc000) { 
    inc = (io_t)0xc000;
  }
#endif
  
  // use saturation
  *pin = op_sadd(*pin, inc);

  if(*pin < sc->inMin) { *pin = sc->inMin; }
  if(*pin > sc->inMax) { *pin = sc->inMax; }

  // FIXME: no customization of output range.
  return scaler_amp_val(sc, *pin);
}
