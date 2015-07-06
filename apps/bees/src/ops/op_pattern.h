#ifndef _OP_PATTERN_H_
#define _OP_PATTERN_H_

#include "types.h"
#include "op.h"
#include "op_math.h"

#define OP_PATTERN_SIZE 64
#define OP_PATTERN_INPUTS 12
#define OP_PATTERN_OUTPUTS 1

//--- op_pattern_t :
typedef struct op_pattern_struct {
  op_t super;
  volatile io_t val;     // output

  volatile io_t in;      // input value (store on write)
  volatile io_t read;    // read position
  volatile io_t write;   // write val to position
  volatile io_t insert;  // insert value as position, shift values up
  volatile io_t rm;      // remove value at position, shift values down
  volatile io_t push;    // push value onto end (sets in?)
  volatile io_t pop;     // pop N values off the end
  volatile io_t start;   // set starting index
  volatile io_t end;     // set ending index
  volatile io_t wrap;    // control whether move,read,write,etc wraps
  volatile io_t pos;     // set position (for move)
  volatile io_t move;    // relative movement and trigger

  volatile io_t * in_val[OP_PATTERN_INPUTS];
  op_out_t outs[OP_PATTERN_OUTPUTS];

  io_t entries[OP_PATTERN_SIZE];

} op_pattern_t;

void op_pattern_init(void* mem);

#endif // header guard
