// asf
#include "print_funcs.h"

// avr32_lib
#include "global.h"

// bees
#include "net_protected.h"
#include "pickle.h"
#include "op_timer.h"
#include "app_timers.h"

//-------------------------------------------------
//----- descriptor
static const char* op_timer_instring = "EVENT\0  ";
static const char* op_timer_outstring = "TIME\0   ";
static const char* op_timer_opstring = "TIMER";

//-------------------------------------------------
//----- static function declaration


// set inputs
static void op_timer_in_event(op_timer_t* timer, const io_t v);
// pickle / unpickle
static u8* op_timer_pickle(op_timer_t* timer, u8* dst);
static u8* op_timer_unpickle(op_timer_t* timer, const u8* src);

// calculate output value
//static void op_timer_calc(op_timer_t* timer); 

// array of input functions 
static op_in_fn op_timer_in[1] = {
  (op_in_fn)&op_timer_in_event,
};

//----- external function definition

/// initialize
void op_timer_init(void* op) {
  op_timer_t* timer = (op_timer_t*)op;

  // superclass functions
  timer->super.in_fn = op_timer_in;
  timer->super.pickle = (op_pickle_fn) (&op_timer_pickle);
  timer->super.unpickle = (op_unpickle_fn) (&op_timer_unpickle);
  
  // superclass state
  timer->super.numInputs = 1;
  timer->super.numOutputs = 1;
  timer->outs[0] = -1;
 
  timer->super.in_val = timer->in_val;
  timer->in_val[0] = &(timer->event);
  timer->super.out = timer->outs;
  timer->super.opString = op_timer_opstring;
  timer->super.inString = op_timer_instring;
  timer->super.outString = op_timer_outstring;
  timer->super.type = eOpTimer;

  // class state
  timer->event = 0;
  timer->ticks = tcTicks;
  timer->interval = 1000;
}

//-------------------------------------------------
//----- static function definition

//===== operator input

// input event
static void op_timer_in_event(op_timer_t* timer, const io_t v) {
  timer->event = v;
  /// FIXME: should check for overflow here...
  timer->interval = tcTicks - timer->ticks;
  timer->ticks = tcTicks;

  // calculate output value in ms convention from libavr32_ticks
  timer->interval = timers_libavr32_tick_to_ms_tick(timer->interval);
  net_activate(timer, 0, timer->interval);

}


// serialization
u8* op_timer_pickle(op_timer_t* timer, u8* dst) {
  // no state
  return dst;
}

u8* op_timer_unpickle(op_timer_t* timer, const u8* src) {
  // no state
  return (u8*)src;
}
