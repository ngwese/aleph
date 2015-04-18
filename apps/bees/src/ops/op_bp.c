
// asf
#include "print_funcs.h"

#include "net_protected.h"
#include "op_bp.h"
#include "op_math.h"

#define CHAN_A 0
#define CHAN_B 1
#define CHAN_C 2
#define CHAN_D 3

#define INDEX_MAX (OP_BP_KEY_COUNT - 1)

//-------------------------------------------------
//---- static func declare
static void op_bp_in_pos(op_bp_t* op, const io_t v);
static void op_bp_in_jump(op_bp_t* op, const io_t v);
static void op_bp_in_wrap(op_bp_t* op, const io_t v);
static void op_bp_in_key(op_bp_t* op, const io_t v);
static void op_bp_in_del(op_bp_t* op, const io_t v);
static void op_bp_in_clear(op_bp_t* op, const io_t v);

static void op_bp_in_a(op_bp_t* op, const io_t v);
static void op_bp_in_b(op_bp_t* op, const io_t v);
static void op_bp_in_c(op_bp_t* op, const io_t v);
static void op_bp_in_d(op_bp_t* op, const io_t v);

static void op_bp_in_mode_a(op_bp_t* op, const io_t v);
static void op_bp_in_mode_b(op_bp_t* op, const io_t v);
static void op_bp_in_mode_c(op_bp_t* op, const io_t v);
static void op_bp_in_mode_d(op_bp_t* op, const io_t v);

// pickle / unpickle
static u8* op_bp_pickle(op_bp_t* op, u8* dst);
static const u8* op_bp_unpickle(op_bp_t* op, const u8* src);

// helpers
static void op_bp_do_clear(op_bp_t* op);
static void op_bp_do_current(op_bp_t* bp);
static void op_bp_do_output(op_bp_t* bp);

// bp_list operations
static void bp_list_insert(bp_list_t* l, const u8 index, const io_t pos, const io_t v);
static void bp_list_pop(bp_list_t* l, const u8 index);
static void bp_list_pop_nearest(bp_list_t* l, const io_t pos);
static void bp_list_move_nearest(bp_list_t* l, const io_t pos);
static void bp_list_merge_nearest(bp_list_t* l, const io_t pos, const io_t v);
static void bp_list_clear(bp_list_t* l);

static bool bp_list_break_between(bp_list_t* l, const io_t pos_a, const io_t pos_b);
static io_t bp_list_interp(bp_list_t* l, const io_t pos, const io_t mode);

//-------------------------------------------------
//---- static vars
static const char* op_bp_instring  = "POS\0    JUMP\0   WRAP\0   KEY\0    DEL\0    CLEAR\0  A\0      B\0      C\0      D\0      MODEA\0  MODEB\0  MODEC\0  MODED\0";
static const char* op_bp_outstring = "A\0      B\0      C\0      D\0      TA\0     TB\0     TC\0     TD\0      ";
static const char* op_bp_opstring  = "BP";

static op_in_fn op_bp_in_fn[14] = {
  (op_in_fn)&op_bp_in_pos,
  (op_in_fn)&op_bp_in_jump,
  (op_in_fn)&op_bp_in_wrap,
  (op_in_fn)&op_bp_in_key,
  (op_in_fn)&op_bp_in_del,
  (op_in_fn)&op_bp_in_clear,
  (op_in_fn)&op_bp_in_a,
  (op_in_fn)&op_bp_in_b, 
  (op_in_fn)&op_bp_in_c,
  (op_in_fn)&op_bp_in_d,
  (op_in_fn)&op_bp_in_mode_a,
  (op_in_fn)&op_bp_in_mode_b, 
  (op_in_fn)&op_bp_in_mode_c,
  (op_in_fn)&op_bp_in_mode_d,
};

//-------------------------------------------------
//---- external func define
void op_bp_init(void* mem) {
  u8 c;
  op_bp_t* op = (op_bp_t*)mem;
  op->super.numInputs = 14;
  op->super.numOutputs = 8;

  for (c = 0; c < op->super.numOutputs; c++) {
    op->outs[c] = -1;
  }

  op->super.in_fn = op_bp_in_fn;
  op->super.in_val = op->in_val;
  op->super.pickle = (op_pickle_fn) (&op_bp_pickle);
  op->super.unpickle = (op_unpickle_fn) (&op_bp_unpickle);

  op->super.out = op->outs;
  op->super.opString = op_bp_opstring;
  op->super.inString = op_bp_instring;
  op->super.outString = op_bp_outstring;
  op->super.type = eOpBP;  

  op->in_val[0] = &(op->pos);
  op->in_val[1] = &(op->jump);
  op->in_val[2] = &(op->wrap);
  op->in_val[3] = &(op->key);
  op->in_val[4] = &(op->del);
  op->in_val[5] = &(op->clear);
  op->in_val[6] = &(op->in[0]);
  op->in_val[7] = &(op->in[1]);
  op->in_val[8] = &(op->in[2]);
  op->in_val[9] = &(op->in[3]);
  op->in_val[10] = &(op->mode[0]);
  op->in_val[11] = &(op->mode[1]);
  op->in_val[12] = &(op->mode[2]);
  op->in_val[13] = &(op->mode[3]);

  op->pos = 0;
  op->jump = 0;
  op->wrap = 0;
  op->key = 0;
  op->del = 0;
  op->clear = 0;
  op->prev_pos = 0;

  for (c = 0; c < OP_BP_CHANS; c++) {
    op->in[c] = 0;
    op->mode[c] = 0;
    op->current[c] = 0;
  }

  op_bp_do_clear(op);
}

//-------------------------------------------------
//---- static bp list func define
static void bp_list_insert(bp_list_t* l, const u8 index, const io_t pos, const io_t v) {
  if (index < 0 || index > INDEX_MAX)
    return; // nothing to do
  // TODO
}

static void bp_list_pop(bp_list_t* l, const u8 index)  {
  if (index < 0 || index > l->last)
    return; // nothing to do
  // TODO
}

static void bp_list_pop_nearest(bp_list_t* l, const io_t pos)  {
  // TODO
}

static void bp_list_move_nearest(bp_list_t* l, const io_t pos)  {
  // TODO
}

static void bp_list_merge_nearest(bp_list_t* l, const io_t pos, const io_t v)  {
  // TODO
}

static void bp_list_clear(bp_list_t* l)  {
  l->last = 0;
  for (u8 i = 0; i < OP_BP_KEY_COUNT; i++) {
    l->keys[i].pos = 0;
    l->keys[i].value = 0;
  }
}

static bool bp_list_break_between(bp_list_t* l, const io_t pos_a, const io_t pos_b) {
  // TODO
  return false;
}

static io_t bp_list_interp(bp_list_t* l, const io_t pos, const io_t mode) {
  // TODO
  return 0;
}


//-------------------------------------------------
//---- static op helper func define

static void op_bp_do_clear(op_bp_t* op) {
  print_dbg("\r\n bp: do_clear");
  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    bp_list_clear(&(op->keys[c]));
  }
  print_dbg(" ...done.");
}

static void op_bp_do_current(op_bp_t* op) {
  // compute and store the current output values
  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    op->current[c] = bp_list_interp(&(op->keys[c]), op->pos, op->mode[c]);
  }
}

static void op_bp_do_output(op_bp_t* op) {
  // FIXME: need to sort these min/max
  io_t low_pos = op->prev_pos;
  io_t high_pos = op->pos;

  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    net_activate(op->outs[c], op->current[c], op);
    if (bp_list_break_between(&(op->keys[c]), low_pos, high_pos)) {
      // MAINT: assumes trigger outs are after value outs
      net_activate(op->outs[c + OP_BP_CHANS], 1, op);
    }
  }
}

//-------------------------------------------------
//---- static input func define

static void op_bp_in_pos(op_bp_t* op, const io_t v) {
  if (v >= OP_MIN_VAL) {
    op->prev_pos = op->pos;
    op->pos = v;
    op_bp_do_current(op);
    op_bp_do_output(op);
  }
}

static void op_bp_in_jump(op_bp_t* op, const io_t v) {
  // range: (-4, 4) specific channel forward or backward
  // 0 input is any channel forward
  if (v >= -OP_BP_CHANS && v <= OP_BP_CHANS) {
    op->jump = v;
  }
  // TODO
}

static void op_bp_in_wrap(op_bp_t* op, const io_t v) {
  if (v > 0) {
    if (op->wrap == 0)
      op->wrap = 1;
  }
  else {
    if (op->wrap > 0)
      op->wrap = 0;
  }
}

static void op_bp_in_key(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel to key (at current pos)
  // 0 is all channels (at current pos)
  if (v >= 0 && v <= OP_BP_CHANS) {
    op->key = v;
  }
  // TODO
}

static void op_bp_in_del(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel breakpoint to delete (at current pos)
  // 0 is all breakpoints (at current pos)
  if (v >= 0 && v <= OP_BP_CHANS) {
    op->del = v;
  }
  // TODO
}

static void op_bp_in_clear(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel to key (at current pos)
  // 0 is all channels (at current pos)
  if (v >= 0 && v <= OP_BP_CHANS) {
    op->clear = v;
    if (v == 0) {
      op_bp_do_clear(op);
    }
    else {
      bp_list_clear(&(op->keys[op->clear]));
    }
  }
}

static void op_bp_in_a(op_bp_t* op, const io_t v) {
  op->in[CHAN_A] = v;
  // TODO
}

static void op_bp_in_b(op_bp_t* op, const io_t v) {
  op->in[CHAN_B] = v;
  // TODO
}

static void op_bp_in_c(op_bp_t* op, const io_t v) {
  op->in[CHAN_C] = v;
  // TODO
}

static void op_bp_in_d(op_bp_t* op, const io_t v) {
  op->in[CHAN_D] = v;
  // TODO
}


static void op_bp_in_mode_a(op_bp_t* op, const io_t v) {
  op->mode[CHAN_A] = 0;
  // TODO
}

static void op_bp_in_mode_b(op_bp_t* op, const io_t v) {
  op->mode[CHAN_B] = 0;
  // TODO
}

static void op_bp_in_mode_c(op_bp_t* op, const io_t v) {
  op->mode[CHAN_C] = 0;
  // TODO
}

static void op_bp_in_mode_d(op_bp_t* op, const io_t v) {
  op->mode[CHAN_D] = 0;
  // TODO
}


// pickle / unpickle

static u8* bp_list_pickle(const bp_list_t* l, u8* start) {
  u8* dst = pickle_io(l->last, start);
  for (u8 i = 0; i < INDEX_MAX; i++) {
    dst = pickle_io(l->keys[i].pos, dst);
    dst = pickle_io(l->keys[i].value, dst);
  }
  return dst;
}

static const u8* bp_list_unpickle(const u8* start, bp_list_t* l) {
  const u8* src = unpickle_io(start, &(l->last));
  for (u8 i = 0; i < INDEX_MAX; i++) {
    src = unpickle_io(src, &(l->keys[i].pos));
    src = unpickle_io(src, &(l->keys[i].value));
  }
  return src;
}

u8* op_bp_pickle(op_bp_t* op, u8* dst) {
  // skipping key and step as they are triggers
  dst = pickle_io(op->pos, dst);
  dst = pickle_io(op->wrap, dst);
  dst = pickle_io(op->in[0], dst);
  dst = pickle_io(op->in[1], dst);
  dst = pickle_io(op->in[2], dst);
  dst = pickle_io(op->in[3], dst);
  dst = pickle_io(op->mode[0], dst);
  dst = pickle_io(op->mode[1], dst);
  dst = pickle_io(op->mode[2], dst);
  dst = pickle_io(op->mode[3], dst);

  // internals
  dst = pickle_io(op->prev_pos, dst);
  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    dst = pickle_io(op->current[c], dst);
    dst = bp_list_pickle(&(op->keys[c]), dst);
  }

  return dst;
}

const u8* op_bp_unpickle(op_bp_t* op, const u8* src) {
  src = unpickle_io(src, &(op->pos));
  src = unpickle_io(src, &(op->wrap));
  src = unpickle_io(src, &(op->in[0]));
  src = unpickle_io(src, &(op->in[1]));
  src = unpickle_io(src, &(op->in[2]));
  src = unpickle_io(src, &(op->in[3]));
  src = unpickle_io(src, &(op->mode[0]));
  src = unpickle_io(src, &(op->mode[1]));
  src = unpickle_io(src, &(op->mode[2]));
  src = unpickle_io(src, &(op->mode[3]));

  // recall internals
  src = unpickle_io(src, &(op->prev_pos));
  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    src = unpickle_io(src, &(op->current[c]));
    src = bp_list_unpickle(src, &(op->keys[c]));
  }

  return src;
}
