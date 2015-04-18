#ifndef _op_frames_H_
#define _op_frames_H_

#include "types.h"
#include "op.h"
#include "op_math.h"

#define OP_FRAMES_SIZE 4
#define OP_FRAMES_LENGTH 32

//--- op_frames_t : keyframe values
typedef struct op_frames_struct {
  op_t super;
  volatile io_t step;
  volatile io_t index;
  volatile io_t key;
  volatile io_t clear;
  volatile io_t acc;
  volatile io_t scrub;
  volatile io_t in[OP_FRAMES_SIZE];
  volatile io_t mute[OP_FRAMES_SIZE];
  volatile io_t * in_val[14]; // step, index, trig, key, mute
  op_out_t outs[4]; // a, b, c, d

  io_t keys[OP_FRAMES_LENGTH][OP_FRAMES_SIZE];

} op_frames_t;

void op_frames_init(void* mem);

#endif // header guard
