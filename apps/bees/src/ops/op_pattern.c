#include "net_protected.h"
#include "op_pattern.h"

#define OP_PATTERN_MAX_POS (OP_PATTERN_SIZE - 1)

//-------------------------------------------------
//---- static func declare
static void op_pattern_in_in(op_pattern_t* op, const io_t v);
static void op_pattern_in_read(op_pattern_t* op, const io_t v);
static void op_pattern_in_write(op_pattern_t* op, const io_t v);
static void op_pattern_in_insert(op_pattern_t* op, const io_t v);
static void op_pattern_in_rm(op_pattern_t* op, const io_t v);
static void op_pattern_in_push(op_pattern_t* op, const io_t v);
static void op_pattern_in_pop(op_pattern_t* op, const io_t v);
static void op_pattern_in_start(op_pattern_t* op, const io_t v);
static void op_pattern_in_end(op_pattern_t* op, const io_t v);
static void op_pattern_in_wrap(op_pattern_t* op, const io_t v);
static void op_pattern_in_pos(op_pattern_t* op, const io_t v);
static void op_pattern_in_move(op_pattern_t* op, const io_t v);

static u8* op_pattern_pickle(op_pattern_t* op, u8* dst);
static const u8* op_pattern_unpickle(op_pattern_t* op, const u8* src);

//-------------------------------------------------
//---- static vars
static const char* op_pattern_instring = "IN\0     READ\0   WRITE\0  INSERT\0 RM\0     PUSH\0   POP\0    START\0  END\0    WRAP\0   POS\0    MOVE\0   ";
static const char* op_pattern_outstring = "VAL\0    ";
static const char* op_pattern_opstring = "pattern";

static op_in_fn op_pattern_in_fn[OP_PATTERN_INPUTS] = {
  (op_in_fn)&op_pattern_in_in,
  (op_in_fn)&op_pattern_in_read,
  (op_in_fn)&op_pattern_in_write,
  (op_in_fn)&op_pattern_in_insert,
  (op_in_fn)&op_pattern_in_rm,
  (op_in_fn)&op_pattern_in_push,
  (op_in_fn)&op_pattern_in_pop,
  (op_in_fn)&op_pattern_in_start,
  (op_in_fn)&op_pattern_in_end,
  (op_in_fn)&op_pattern_in_wrap,
  (op_in_fn)&op_pattern_in_pos,
  (op_in_fn)&op_pattern_in_move
};

//-------------------------------------------------
//---- external func define
void op_pattern_init(void* mem) {
  op_pattern_t* pattern = (op_pattern_t*)mem;
  pattern->super.numInputs = OP_PATTERN_INPUTS;
  pattern->super.numOutputs = OP_PATTERN_OUTPUTS;
  pattern->outs[0] = -1;

  pattern->super.in_fn = op_pattern_in_fn;
  pattern->super.in_val = pattern->in_val;
  pattern->super.pickle = (op_pickle_fn)(&op_pattern_pickle);
  pattern->super.unpickle = (op_unpickle_fn)(&op_pattern_unpickle);

  pattern->super.out = pattern->outs;
  pattern->super.opString = op_pattern_opstring;
  pattern->super.inString = op_pattern_instring;
  pattern->super.outString = op_pattern_outstring;
  pattern->super.type = eOpPattern;

  pattern->in_val[0] = &(pattern->in);
  pattern->in_val[1] = &(pattern->read);
  pattern->in_val[2] = &(pattern->write);
  pattern->in_val[3] = &(pattern->insert);
  pattern->in_val[4] = &(pattern->rm);
  pattern->in_val[5] = &(pattern->push);
  pattern->in_val[6] = &(pattern->pop);
  pattern->in_val[7] = &(pattern->start);
  pattern->in_val[8] = &(pattern->end);
  pattern->in_val[9] = &(pattern->wrap);
  pattern->in_val[10] = &(pattern->pos);
  pattern->in_val[11] = &(pattern->move);

  pattern->in = 0;
  pattern->read = 0;
  pattern->write = 0;
  pattern->insert = 0;
  pattern->rm = 0;
  pattern->push = 0;
  pattern->pop = 0;
  pattern->start = 0;
  pattern->end = OP_PATTERN_MAX_POS;
  pattern->wrap = 0;
  pattern->pos = 0;
  pattern->move = 0;
}

//-------------------------------------------------
//---- static func define

static io_t op_pattern_get_idx(op_pattern_t* op, const io_t pos) {
  io_t idx;

  if (op->wrap) {
    // FIXME: implement this
    if (pos < op->start) {
      
    }
    idx = 0;
  }
  else {
    // clamp to [start,end]
    if (pos > op->end)
      idx = op->end;
    else if (pos < op->start)
      idx = op->start;
    else
      idx = pos;
  }

  return idx;
}

static void op_pattern_in_in(op_pattern_t* op, const io_t val) {
  op->in = val;
}

static void op_pattern_in_read(op_pattern_t* op, const io_t pos) {
  op->val = op->entries[op_pattern_get_idx(op, pos)];
  net_activate(op->outs[0], op->val, op);
}

static void op_pattern_in_write(op_pattern_t* op, const io_t pos) {
  op->entries[op_pattern_get_idx(op, pos)] = op->in;
}

static void op_pattern_in_insert(op_pattern_t* op, const io_t pos) {
  // TODO
}

static void op_pattern_in_rm(op_pattern_t* op, const io_t pos) {
  // TODO
}

static void op_pattern_in_push(op_pattern_t* op, const io_t v) {
  // TODO
}

static void op_pattern_in_pop(op_pattern_t* op, const io_t num) {
  // TODO
}

static void op_pattern_in_start(op_pattern_t* op, const io_t pos) {
  if (pos < 0)
    op->start = 0;
  else if (pos >= op->end)
    op->start = op->end - 1;
  else
    op->start = pos;
}

static void op_pattern_in_end(op_pattern_t* op, const io_t pos) {
  if (pos > OP_PATTERN_MAX_POS)
    op->end = OP_PATTERN_MAX_POS;
  else if (pos <= op->start)
    op->end = op->start + 1;
  else
    op->end = pos;
}

static void op_pattern_in_wrap(op_pattern_t* op, const io_t v) {
  if (v > 0)
    op->wrap = 1;
  else
    op->wrap = 0;
}

static void op_pattern_in_pos(op_pattern_t* op, const io_t pos) {
  if (pos < 0)
    op->pos = 0;
  else if (pos > OP_PATTERN_MAX_POS)
    op->pos = OP_PATTERN_MAX_POS;
  else
    op->pos = pos;
}

static void op_pattern_in_move(op_pattern_t* op, const io_t v) {
  op_pattern_in_read(op, op_pattern_get_idx(op, op->pos + v));
}

// pickle / unpickle
u8* op_pattern_pickle(op_pattern_t* op, u8* dst) {
  u8 i;

  dst = pickle_io(op->val, dst);
  dst = pickle_io(op->in, dst);
  // skip read
  // skip write
  // skip insert
  // skip rm
  // skip push
  // skip pop
  dst = pickle_io(op->start, dst);
  dst = pickle_io(op->end, dst);
  dst = pickle_io(op->wrap, dst);
  dst = pickle_io(op->pos, dst);
  // skip move

  for(i=0; i<OP_PATTERN_SIZE; ++i) {
    dst = pickle_io(op->entries[i], dst);
  }

  return dst;
}

const u8* op_pattern_unpickle(op_pattern_t* op, const u8* src) {
  u8 i;

  src = unpickle_io(src, &(op->val));
  src = unpickle_io(src, &(op->in));
  src = unpickle_io(src, &(op->start));
  src = unpickle_io(src, &(op->end));
  src = unpickle_io(src, &(op->wrap));
  src = unpickle_io(src, &(op->pos));

  for(i=0; i<OP_PATTERN_SIZE; ++i) {
    src = unpickle_io(src, &(op->entries[i]));
  }

  return src;
}
