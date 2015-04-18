#ifndef _op_bp_H_
#define _op_bp_H_

#include "types.h"
#include "op.h"
#include "op_math.h"

#define OP_BP_CHANS 4
#define OP_BP_KEY_COUNT 5

typedef struct bp_value_struct {
  io_t pos;
  io_t value;
} bp_value_t;

typedef struct bp_list_struct {
  u8 last;
  //io_t clear_value;
  bp_value_t keys[OP_BP_KEY_COUNT];
} bp_list_t;

//--- op_bp_t : breakpoint (bp) operator
typedef struct op_bp_struct {
  op_t super;

  volatile io_t pos;
  volatile io_t jump;
  volatile io_t wrap;
  volatile io_t key;
  volatile io_t del;
  volatile io_t clear;
  volatile io_t min;
  volatile io_t max;
  volatile io_t clip;
  volatile io_t in[OP_BP_CHANS];
  volatile io_t mode[OP_BP_CHANS];
  volatile io_t * in_val[17]; // step, index, trig, key, mute

  op_out_t outs[8]; // a, b, c, d, ta, tb, tc, td

  // data
  io_t      prev_pos;
  io_t      current[OP_BP_CHANS];
  bp_list_t keys[OP_BP_CHANS];

} op_bp_t;

void op_bp_init(void* mem);

#endif // header guard
