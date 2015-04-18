
// asf
#include "print_funcs.h"

#include "net_protected.h"
#include "op_frames.h"
#include "op_math.h"

#define CHAN_A 0
#define CHAN_B 1
#define CHAN_C 2
#define CHAN_D 3

#define INDEX_MAX (OP_FRAMES_LENGTH - 1)

//-------------------------------------------------
//---- static func declare
static void op_frames_in_a(op_frames_t* frames, const io_t v);
static void op_frames_in_b(op_frames_t* frames, const io_t v);
static void op_frames_in_c(op_frames_t* frames, const io_t v);
static void op_frames_in_d(op_frames_t* frames, const io_t v);
static void op_frames_in_step(op_frames_t* frames, const io_t v);
static void op_frames_in_index(op_frames_t* frames, const io_t v);
static void op_frames_in_key(op_frames_t* frames, const io_t v);
static void op_frames_in_clear(op_frames_t* frames, const io_t v);
static void op_frames_in_ma(op_frames_t* frames, const io_t v);
static void op_frames_in_mb(op_frames_t* frames, const io_t v);
static void op_frames_in_mc(op_frames_t* frames, const io_t v);
static void op_frames_in_md(op_frames_t* frames, const io_t v);
static void op_frames_in_acc(op_frames_t* frames, const io_t v);
static void op_frames_in_scrub(op_frames_t* frames, const io_t v);

// pickle / unpickle
static u8* op_frames_pickle(op_frames_t* op, u8* dst);
static const u8* op_frames_unpickle(op_frames_t* op, const u8* src);

// helpers
static void maybe_key(op_frames_t* frames, const io_t v, const u8 chan);
static void do_clear(op_frames_t* frames, const io_t v);
static void do_mute(op_frames_t* frames, const io_t v, const u8 chan);

//-------------------------------------------------
//---- static vars
static const char* op_frames_instring = "INDEX\0  STEP\0   SCRUB\0  KEY\0    CLEAR\0  ACC\0    A\0      B\0      C\0      D\0      MUTEA\0  MUTEB\0  MUTEC\0  MUTED\0";
static const char* op_frames_outstring = "A\0      B\0      C\0      D\0      ";
static const char* op_frames_opstring = "FRAMES";

static op_in_fn op_frames_in_fn[14] = {
  (op_in_fn)&op_frames_in_index,
  (op_in_fn)&op_frames_in_step,
  (op_in_fn)&op_frames_in_scrub,
  (op_in_fn)&op_frames_in_key,
  (op_in_fn)&op_frames_in_clear,
  (op_in_fn)&op_frames_in_acc,
  (op_in_fn)&op_frames_in_a,
  (op_in_fn)&op_frames_in_b, 
  (op_in_fn)&op_frames_in_c,
  (op_in_fn)&op_frames_in_d,
  (op_in_fn)&op_frames_in_ma,
  (op_in_fn)&op_frames_in_mb, 
  (op_in_fn)&op_frames_in_mc,
  (op_in_fn)&op_frames_in_md,
};

//-------------------------------------------------
//---- external func define
void op_frames_init(void* mem) {
  u8 c;
  op_frames_t* frames = (op_frames_t*)mem;
  frames->super.numInputs = 14;
  frames->super.numOutputs = 4;

  for (c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->outs[c] = -1;
  }

  frames->super.in_fn = op_frames_in_fn;
  frames->super.in_val = frames->in_val;
  frames->super.pickle = (op_pickle_fn) (&op_frames_pickle);
  frames->super.unpickle = (op_unpickle_fn) (&op_frames_unpickle);

  frames->super.out = frames->outs;
  frames->super.opString = op_frames_opstring;
  frames->super.inString = op_frames_instring;
  frames->super.outString = op_frames_outstring;
  frames->super.type = eOpFrames;  

  frames->in_val[0] = &(frames->index);
  frames->in_val[1] = &(frames->step);
  frames->in_val[2] = &(frames->scrub);
  frames->in_val[3] = &(frames->key);
  frames->in_val[4] = &(frames->clear);
  frames->in_val[5] = &(frames->acc);
  frames->in_val[6] = &(frames->in[0]);
  frames->in_val[7] = &(frames->in[1]);
  frames->in_val[8] = &(frames->in[2]);
  frames->in_val[9] = &(frames->in[3]);
  frames->in_val[10] = &(frames->mute[0]);
  frames->in_val[11] = &(frames->mute[1]);
  frames->in_val[12] = &(frames->mute[2]);
  frames->in_val[13] = &(frames->mute[3]);


  frames->index = 0;
  frames->step = 0;
  frames->scrub = 0;
  frames->key = 0;
  frames->clear = 0;
  frames->acc = 0;
  for (c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->in[c] = 0;
  }
  for (c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->mute[c] = 0;
  }

  do_clear(frames, 0);
}

//-------------------------------------------------
//---- static func define

static void maybe_key(op_frames_t* frames, const io_t v, const u8 chan) {
  if (frames->key != 0) {
    print_dbg("\r\n FRAMES: maybe_key");
    frames->keys[frames->index][chan] = v;
    print_dbg(" Ox");
    print_dbg_hex(frames->index);
    print_dbg(" Ox");
    print_dbg_hex(chan);
    print_dbg(" Ox");
    print_dbg_hex(v);
    net_activate(frames->outs[chan], v, frames);
  }   
}

static void do_in_value(op_frames_t* frames, const io_t v, const u8 chan) {
  if (frames->acc == 1) {
    frames->in[chan] = op_sadd(frames->in[chan], v); // based on op_accum
    //frames->in[chan] += v
  }
  else {
      frames->in[chan] = v;
  }
  maybe_key(frames, frames->in[chan], chan);
}

static void do_clear(op_frames_t* frames, const io_t v) {
  print_dbg("\r\n FRAMES: do_clear");
  for (u8 s = 0; s < OP_FRAMES_LENGTH; s++) {
    for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
      frames->keys[s][c] = v;
    }
  }
  print_dbg(" ...done.");
}

static void op_frames_in_acc(op_frames_t* frames, const io_t v) {
  if (v > 0) {
    if (frames->acc == 0)
      frames->acc = 1;
  }
  else {
    if (frames->acc > 0)
      frames->acc = 0;
  }
}

static void op_frames_in_a(op_frames_t* frames, const io_t v) {
  do_in_value(frames, v, CHAN_A);
}

static void op_frames_in_b(op_frames_t* frames, const io_t v) {
  do_in_value(frames, v, CHAN_B);
}

static void op_frames_in_c(op_frames_t* frames, const io_t v) {
  do_in_value(frames, v, CHAN_C);
}

static void op_frames_in_d(op_frames_t* frames, const io_t v) {
  do_in_value(frames, v, CHAN_D);
}

static void do_mute(op_frames_t* frames, const io_t v, const u8 chan) {
  if (v > 0) {
    if (frames->mute[chan] == 0)
      frames->mute[chan] = 1;
  }
  else {
    if (frames->mute[chan] > 0)
      frames->mute[chan] = 0;
  }
}

static void op_frames_in_ma(op_frames_t* frames, const io_t v) {
  do_mute(frames, v, CHAN_A);
}

static void op_frames_in_mb(op_frames_t* frames, const io_t v) {
  do_mute(frames, v, CHAN_B);
}

static void op_frames_in_mc(op_frames_t* frames, const io_t v) {
  do_mute(frames, v, CHAN_C);
}

static void op_frames_in_md(op_frames_t* frames, const io_t v) {
  do_mute(frames, v, CHAN_D);
}

static void op_frames_in_key(op_frames_t* frames, const io_t v) {
  print_dbg("\r\n FRAMES: op_frames_in_key");
  if (v > 0) {
    if (frames->key == 0) {
      frames->key = 1;
      print_dbg(" at: 0x");
      print_dbg_hex(frames->index);
      // store abcd values at index
      // for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
      //   frames->keys[frames->index][c] = frames->in[c];
      //   print_dbg(" 0x");
      //   print_dbg_hex(frames->in[c]);
      // }
    }
  }
  else {
    if (frames->key > 0)
      frames->key = 0;
  }
}			      

static void op_frames_in_step(op_frames_t* frames, const io_t v) {
  if (v < -INDEX_MAX) {
    frames->step = -INDEX_MAX;
  }
  else if (v > INDEX_MAX) {
    frames->step = INDEX_MAX;
  }
  else {
    frames->step = v;
  }
  
  // move to next index based on step
  io_t next_idx = (frames->index + frames->step) % OP_FRAMES_LENGTH;
  if (next_idx < 0) {
    next_idx = INDEX_MAX + next_idx;
  }
  print_dbg("\r\n FRAMES: op_frames_in_step, next: 0x");
  print_dbg_hex(next_idx);
  op_frames_in_index(frames, next_idx);
}

static void op_frames_in_index(op_frames_t* frames, const io_t v) {
  if (v > INDEX_MAX) {
    frames->index = INDEX_MAX;
  }
  else if (v < 0) {
    frames->index = 0;
  }
  else {
    frames->index = v;
  }

  print_dbg("\r\n FRAMES: op_frames_in_index, i: 0x");
  print_dbg_hex(frames->index);

  for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
    print_dbg(" 0x");
    print_dbg_hex(frames->keys[frames->index][c]);
    net_activate(frames->outs[c], frames->keys[frames->index][c], frames);
  }
}

static void op_frames_in_scrub(op_frames_t* frames, const io_t v) {
  if (v < 0) {
    frames->scrub = 0;
  }
  else {
    frames->scrub = v;
  }
  // TODO
  // interp values between frames; with domain OP_MIN_VAL to OP_MAX_VAL
}

static void op_frames_in_clear(op_frames_t* frames, const io_t v) {
  if (v > 0) {
    if (frames->clear == 0)
      frames->clear = 1;
  }
  else {
    if (frames->clear > 0)
      frames->clear = 0;
  }

  if (frames->clear) {
    do_clear(frames, 0);
  }
}


// pickle / unpickle
u8* op_frames_pickle(op_frames_t* op, u8* dst) {
  // skipping key and step as they are triggers
  dst = pickle_io(op->index, dst);
  dst = pickle_io(op->scrub, dst);
  dst = pickle_io(op->acc, dst);
  dst = pickle_io(op->in[0], dst);
  dst = pickle_io(op->in[1], dst);
  dst = pickle_io(op->in[2], dst);
  dst = pickle_io(op->in[3], dst);
  dst = pickle_io(op->mute[0], dst);
  dst = pickle_io(op->mute[1], dst);
  dst = pickle_io(op->mute[2], dst);
  dst = pickle_io(op->mute[3], dst);

  // store keys
  for (u8 s = 0; s < OP_FRAMES_LENGTH; s++) {
    for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
      dst = pickle_io(op->keys[s][c], dst);
    }
  }

  return dst;
}

const u8* op_frames_unpickle(op_frames_t* op, const u8* src) {
  src = unpickle_io(src, &(op->index));
  src = unpickle_io(src, &(op->scrub));
  src = unpickle_io(src, &(op->acc));
  src = unpickle_io(src, &(op->in[0]));
  src = unpickle_io(src, &(op->in[1]));
  src = unpickle_io(src, &(op->in[2]));
  src = unpickle_io(src, &(op->in[3]));
  src = unpickle_io(src, &(op->mute[0]));
  src = unpickle_io(src, &(op->mute[1]));
  src = unpickle_io(src, &(op->mute[2]));
  src = unpickle_io(src, &(op->mute[3]));

  // recall keys
  for (u8 s = 0; s < OP_FRAMES_LENGTH; s++) {
    for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
      src = unpickle_io(src, &(op->keys[s][c]));
    }
  }

  return src;
}
