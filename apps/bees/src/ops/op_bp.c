
// asf
#include "print_funcs.h"

#include "net_protected.h"
#include "op_bp.h"
#include "op_math.h"

#define CHAN_A 0
#define CHAN_B 1
#define CHAN_C 2
#define CHAN_D 3

#define CHAN_A_MSK (1 << CHAN_A)
#define CHAN_B_MSK (1 << CHAN_B)
#define CHAN_C_MSK (1 << CHAN_C)
#define CHAN_D_MSK (1 << CHAN_D)

#define CHAN_ALL_MSK (CHAN_A_MSK | CHAN_B_MSK | CHAN_C_MSK | CHAN_D_MSK)

#define INDEX_MAX (OP_BP_KEY_COUNT - 1)

//-------------------------------------------------
//---- static func declare
static void op_bp_in_pos(op_bp_t* op, const io_t v);
static void op_bp_in_jump(op_bp_t* op, const io_t v);
static void op_bp_in_wrap(op_bp_t* op, const io_t v);
static void op_bp_in_key(op_bp_t* op, const io_t v);
static void op_bp_in_del(op_bp_t* op, const io_t v);
static void op_bp_in_clear(op_bp_t* op, const io_t v);
static void op_bp_in_min(op_bp_t* op, const io_t v);
static void op_bp_in_max(op_bp_t* op, const io_t v);
static void op_bp_in_clip(op_bp_t* op, const io_t v);

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
static u8 bp_list_insert(bp_list_t* l, const io_t pos, const io_t v);
static void bp_list_pop_nearest(bp_list_t* l, const io_t pos);
static u8 bp_list_replace_nearest(bp_list_t* l, const io_t pos, const io_t v);
static void bp_list_merge_nearest(bp_list_t* l, const io_t pos, const io_t v);
static void bp_list_clear(bp_list_t* l);
static bool bp_list_is_empty(bp_list_t* l);
static bool bp_list_is_full(bp_list_t* l);
static s8 bp_list_lower_index(bp_list_t* l, const io_t pos);
static s8 bp_list_nearest_index(bp_list_t* l, const io_t pos);

static bool bp_list_break_between(bp_list_t* l, const io_t pos_a, const io_t pos_b);
static io_t bp_list_interp(bp_list_t* l, const io_t pos, const io_t mode, const bool wrap);

//-------------------------------------------------
//---- static vars
static const char* op_bp_instring  = "POS\0    JUMP\0   WRAP\0   KEY\0    DEL\0    CLEAR\0  MIN\0    MAX\0     CLIP\0   A\0      B\0      C\0      D\0      MODEA\0  MODEB\0  MODEC\0  MODED\0";
static const char* op_bp_outstring = "A\0      B\0      C\0      D\0      TA\0     TB\0     TC\0     TD\0      ";
static const char* op_bp_opstring  = "BP";

static op_in_fn op_bp_in_fn[17] = {
  (op_in_fn)&op_bp_in_pos,
  (op_in_fn)&op_bp_in_jump,
  (op_in_fn)&op_bp_in_wrap,
  (op_in_fn)&op_bp_in_key,
  (op_in_fn)&op_bp_in_del,
  (op_in_fn)&op_bp_in_clear,
  (op_in_fn)&op_bp_in_min,
  (op_in_fn)&op_bp_in_max,
  (op_in_fn)&op_bp_in_clip,
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
  op->super.numInputs = 17;
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
  op->in_val[6] = &(op->min);
  op->in_val[7] = &(op->max);
  op->in_val[8] = &(op->clip);
  op->in_val[9] = &(op->in[0]);
  op->in_val[10] = &(op->in[1]);
  op->in_val[11] = &(op->in[2]);
  op->in_val[12] = &(op->in[3]);
  op->in_val[13] = &(op->mode[0]);
  op->in_val[14] = &(op->mode[1]);
  op->in_val[15] = &(op->mode[2]);
  op->in_val[16] = &(op->mode[3]);

  op->pos = 0;
  op->jump = 0;
  op->wrap = 0;
  op->key = 0;
  op->del = 0;
  op->clear = 0;
  op->min = OP_MIN_VAL;
  op->max = OP_MAX_VAL;
  op->clip = 0;
  op->prev_pos = 0;

  for (c = 0; c < OP_BP_CHANS; c++) {
    op->in[c] = 0;
    op->mode[c] = 0;
    op->current[c] = 0;
  }

  // TODO: allow for clearing to an initial value (say OP_MAX_VAL / 2 for unipolar CV usage)
  op_bp_do_clear(op);
}

//-------------------------------------------------
//---- static bp list func define

static inline io_t bp_list_pos_at(bp_list_t* l, const u8 idx) {
  return l->keys[idx].pos;
}

static inline io_t bp_list_value_at(bp_list_t* l, const u8 idx) {
  return l->keys[idx].value;
}

static inline bool bp_list_is_full(bp_list_t* l) {
  return l->last == INDEX_MAX;
}

static inline bool bp_list_is_empty(bp_list_t* l) {
  return l->last == -1;
}

static inline void bp_list_clear_index(bp_list_t* l, const u8 index) {
  l->keys[index].pos = OP_MAX_VAL;
  l->keys[index].value = 0;
}

static void bp_list_clear(bp_list_t* l)  {
  l->last = -1;
  for (u8 i = 0; i < OP_BP_KEY_COUNT; i++) {
    bp_list_clear_index(l, i);
  }
}

static u8 bp_list_insert(bp_list_t* l, const io_t pos, const io_t v) {
  // inserts the given break point, pushing existing values up and out
  if (bp_list_is_empty(l)) {
    // empty; insert at head
    l->last = 0;
    l->keys[0].pos = pos;
    l->keys[0].value = v;
    print_dbg("\r\n    bp: list_insert at head (was empty)");
  }
  else {
    u8 lower = bp_list_lower_index(l, pos);
    if (lower == l->last && l->last < INDEX_MAX) {
      // tail; append
      l->last++;
      l->keys[l->last].pos = pos;
      l->keys[l->last].value = v;
      print_dbg("\r\n    bp: list_insert append to tail, last: 0x");
      print_dbg_hex(l->last);
    }
    else if (lower == INDEX_MAX) {
      // tail; hit capacity etc, just reset tail, last stays the same
      l->keys[INDEX_MAX].pos = pos;
      l->keys[INDEX_MAX].value = v;
      print_dbg("\r\n    bp: list_insert reset tail (max index), last: 0x");
      print_dbg_hex(l->last);
    }
    else {
      // middle, shuffle keys up
      u8 upper = l->last < INDEX_MAX ? l->last : INDEX_MAX;
      for (u8 i = lower + 1; i <= upper; i++) { // FIXME: < upper or <= upper?
        l->keys[i].pos = l->keys[i + 1].pos;
        l->keys[i].value = l->keys[i + 1].value;
      }
      // insert key
      l->keys[lower].pos = pos;
      l->keys[lower].value = v;
      l->last++;
      print_dbg("\r\n    bp: list_insert middle i: 0x");
      print_dbg_hex(lower);
      print_dbg(" last: 0x");
      print_dbg_hex(l->last);
    }
  }
  return l->last;
}

static s8 bp_list_lower_index(bp_list_t* l, const io_t pos) {
  s8 i = -1;

  // search list return index of breakpoint just lower than pos
  if (bp_list_is_empty(l)) {
    i = -1;
  }
  else {
    i = 0; // MAINT: check this logic
    for (; i <= l->last; i++)
      if (pos < bp_list_pos_at(l, i))
        break;
  }

  print_dbg("\r\n    bp: list_lower_index, pos: 0x");
  print_dbg_hex(pos);
  print_dbg(" i: 0x");
  print_dbg_hex(i);

  return i;
}

static s8 bp_list_nearest_index(bp_list_t *l, const io_t pos) {
  s8 idx = bp_list_lower_index(l, pos);
  if (idx == -1)
    // nothing in the list
    return -1;
  if (l->last == 0)
    // one thing in the list
    return idx;
  if (l->last == idx)
    // already at the end
    return idx;
  // 
  io_t dlow = pos - bp_list_pos_at(l, idx);
  io_t dhigh = bp_list_pos_at(l, idx + 1) - pos;
  if (dhigh < dlow)
    // snap high
    return idx + 1;
  
  return idx;
}

static void bp_list_pop_nearest(bp_list_t* l, const io_t pos)  {
  if (!bp_list_is_empty(l)) {
    s8 index = bp_list_nearest_index(l, pos);
    if (index != l->last) {
      // ripple down
      for (u8 i = index; i < l->last; i++) {
        l->keys[i].pos = l->keys[i + 1].pos;
        l->keys[i].value = l->keys[i + 1].value;
      }
    }
    bp_list_clear_index(l, l->last);
    l->last--;
  }
}

static u8 bp_list_replace_nearest(bp_list_t* l, const io_t pos, const io_t v)  {
  // FIXME: doesn't handle -1 return from nearest
  u8 idx = bp_list_nearest_index(l, pos);
  l->keys[idx].pos = pos;
  l->keys[idx].value = v;
  return idx;
}

static void bp_list_merge_nearest(bp_list_t* l, const io_t pos, const io_t v)  {
  if (bp_list_is_full(l)) {
    u8 idx = bp_list_replace_nearest(l, pos, v);
    print_dbg("\r\n    bp: list_merge; replaced nearest 0x");
    print_dbg_hex(idx);
  }
  else {
    bp_list_insert(l, pos, v);
  }
}

static bool bp_list_break_between(bp_list_t* l, const io_t pos_a, const io_t pos_b) {
  // TODO
  return false;
}

static io_t bp_list_interp(bp_list_t* l, const io_t pos, const io_t mode, const bool wrap) {
  // TODO: handle mode
  io_t p1, v1, p2, v2, out;

  if (bp_list_is_empty(l)) {
    print_dbg("\r\n    bp: list_interp; is empty");
    return 0; // this should match the default key value
  }

  s8 lower = bp_list_lower_index(l, pos);
  if (lower < 0) { // before first key
    if (wrap) lower = l->last; // grab highest index
    else {
      out = bp_list_value_at(l, 0); // no wrap; hold the value of the first keys
      print_dbg("\r\n    bp: list_interp; hold first => 0x");
      print_dbg_hex(out);
      return out;
    } 
  }
  p1 = bp_list_pos_at(l, lower);
  v1 = bp_list_value_at(l, lower);

  u8 higher = lower + 1;
  if (higher > l->last) { // after last key
    if (wrap) higher = 0; // grab lowest index
    else {
      out = bp_list_value_at(l, l->last); // no wrap; hold value of last key
      print_dbg("\r\n    bp: list_interp; hold last => 0x");
      print_dbg_hex(out);
      return out;
    }
  }
  p2 = bp_list_pos_at(l, higher);
  v2 = bp_list_value_at(l, higher);

  // FIXME: this doesn't work if pos is not between p1 and p2 (i.e. one or both wrapped, test the one key case)
  out = v1 + (v2 - v1) * ((pos - p1) / (p2 - p1));
  print_dbg("\r\n    bp: list_interp; interp => 0x");
  print_dbg_hex(out);
  return out;
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

static inline io_t op_bp_clip_value(op_bp_t *op, io_t v) {
  return v < op->min ? op->min : (v > op->max ? op->max : v);
}

static void op_bp_do_current(op_bp_t* op) {
  // compute and store the current output values
  bool wrap = op->wrap == 1;
  for (u8 c = 0; c < OP_BP_CHANS; c++) {
    print_dbg("\r\n bp: do_current");
    print_dbg(" c: 0x");
    print_dbg_hex(c);
    io_t v = bp_list_interp(&(op->keys[c]), op->pos, op->mode[c], wrap);
    op->current[c] = op->clip ? op_bp_clip_value(op, v) : v;
    print_dbg("\r\n...current is 0x:");
    print_dbg_hex(op->current[c]);
  }
}

static void op_bp_do_output(op_bp_t* op) {
  print_dbg("\r\n bp: do_output\r\n");

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
    print_dbg("\r\n bp: in_pos, prev: 0x");
    print_dbg_hex(op->prev_pos);
    print_dbg(" pos: 0x");
    print_dbg_hex(op->pos);
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

static bool op_bp_do_key(op_bp_t* op, const u8 chan) {
  print_dbg("\r\n bp: do_key");

  bp_list_t* l = &(op->keys[chan]);
  io_t out_val;
  bool did_key;

  if (bp_list_is_empty(l)) {
    print_dbg(" empty, first key");
    out_val = op->in[chan];
    bp_list_insert(l, op->pos, out_val);
    did_key = true;
  }
  else {
    s8 idx = bp_list_nearest_index(l, op->pos);
    if (idx >= 0 && bp_list_pos_at(l, idx) == op->pos) {
    // we are on a key, just modify it
      out_val = op_sadd(l->keys[idx].value, op->in[chan]);
      l->keys[idx].value = out_val;
      did_key = false;
      print_dbg(" on key; acc: 0x");
      print_dbg_hex(out_val);
    }
    else {
    // we are not on a key, just insert/move it keeping the value at this position the same
      print_dbg(" insert/move key");
      out_val = bp_list_interp(l, op->pos, op->mode[chan], op->wrap);
      bp_list_merge_nearest(l, op->pos, out_val);
      did_key = true;
    }
  }
  
  op->current[chan] = out_val;
  print_dbg("\r\n bp: do_key done; outputting: 0x");
  print_dbg_hex(op->current[chan]);
  net_activate(op->outs[chan], op->current[chan], op); // MAINT: should this trigger?

  return did_key;
}

static void op_bp_in_key(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel to key (at current pos)
  // 0 is all channels (at current pos)
  if (v >= 0 && v <= CHAN_ALL_MSK) {
    print_dbg("\r\n bp: in_key setting key mask to: 0x");
    print_dbg_hex(v);
    op->key = v;
  }
  if (op->key > 0) {
    for (u8 c = 0; c < OP_BP_CHANS; c++) {
      if (op->key & (1 << c)) {
        print_dbg("\r\n bp: in_key doing c: 0x");
        print_dbg_hex(c);
        op_bp_do_key(op, c);
      }
    }
  }
}

static void op_bp_do_del(op_bp_t* op, const u8 chan) {
  bp_list_t* l = &(op->keys[chan]);

  bp_list_pop_nearest(l, op->pos);
  op->current[chan] = bp_list_interp(l, op->pos, op->mode[chan], op->wrap);  
  net_activate(op->outs[chan], op->current[chan], op); // MAINT: should this trigger?
}

static void op_bp_in_del(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel breakpoint to delete (at current pos)
  // 0 is all breakpoints (at current pos)
  if (v >= 0 && v <= CHAN_ALL_MSK) {
    print_dbg("\r\n bp: in_del setting del mask to: 0x");
    print_dbg_hex(v);
    op->del = v;
  }
  if (op->del > 0) {
    for (u8 c = 0; c < OP_BP_CHANS; c++) {
      if (op->del & (1 << c)) {
        print_dbg("\r\n bp: in_del doing c: 0x");
        print_dbg_hex(c);
        op_bp_do_del(op, c);
      }
    }
  }
}

static void op_bp_in_clear(op_bp_t* op, const io_t v) {
  // range: (0, 4) specific channel to key (at current pos)
  // 0 is all channels (at current pos)
  if (v >= 0 && v <= CHAN_ALL_MSK) {
    op->clear = v;
    if (op->clear > 0) {
      for (u8 c = 0; c < OP_BP_CHANS; c++) {
        if (op->clear & (1 << c)) {
          print_dbg("\r\n bp: in_clear doing c: 0x");
          print_dbg_hex(c);
          bp_list_clear(&(op->keys[c]));
        }
      }
    }
  }
}

static void op_bp_in_min(op_bp_t* op, const io_t v) {
  op->min = v;
  if (op->min >= op->max) {
    op->min = op->max - 1;
  }
}

static void op_bp_in_max(op_bp_t* op, const io_t v) {
  op->min = v;
  if (op->min >= op->max) {
    op->max = op->min + 1;
  }
}

static void op_bp_in_clip(op_bp_t* op, const io_t v) {
  if (v > 0) {
    if (op->clip == 0)
      op->clip = 1;
  }
  else {
    if (op->clip > 0)
      op->clip = 0;
  }
}

static void op_bp_in_a(op_bp_t* op, const io_t v) {
  print_dbg("\r\n bp: in_a v: 0x");
  print_dbg_hex(v);
  op->in[CHAN_A] = v;
  if (op->key & CHAN_A_MSK) op_bp_do_key(op, CHAN_A);
}

static void op_bp_in_b(op_bp_t* op, const io_t v) {
  print_dbg("\r\n bp: in_b v: 0x");
  print_dbg_hex(v);
  op->in[CHAN_B] = v;
  if (op->key & CHAN_B_MSK) op_bp_do_key(op, CHAN_B);
}

static void op_bp_in_c(op_bp_t* op, const io_t v) {
  print_dbg("\r\n bp: in_c v: 0x");
  print_dbg_hex(v);
  op->in[CHAN_C] = v;
  if (op->key & CHAN_C_MSK) op_bp_do_key(op, CHAN_C);
}

static void op_bp_in_d(op_bp_t* op, const io_t v) {
  print_dbg("\r\n bp: in_d v: 0x");
  print_dbg_hex(v);
  op->in[CHAN_D] = v;
  if (op->key & CHAN_D_MSK) op_bp_do_key(op, CHAN_D);
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
  //dst = pickle_io(l->clear_value, dst);
  for (u8 i = 0; i < INDEX_MAX; i++) {
    dst = pickle_io(l->keys[i].pos, dst);
    dst = pickle_io(l->keys[i].value, dst);
  }
  return dst;
}

static const u8* bp_list_unpickle(const u8* start, bp_list_t* l) {
  const u8* src = unpickle_io(start, &(l->last));
  //src = unpickle_io(src, &(l->clear_value));
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
  dst = pickle_io(op->min, dst);
  dst = pickle_io(op->max, dst);
  dst = pickle_io(op->clip, dst);
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
  src = unpickle_io(src, &(op->min));
  src = unpickle_io(src, &(op->max));
  src = unpickle_io(src, &(op->clip));
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
