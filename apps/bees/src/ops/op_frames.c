#include "net_protected.h"
#include "op_frames.h"

//-------------------------------------------------
//---- static func declare
static void op_frames_in_a(op_frames_t* frames, const io_t v);
static void op_frames_in_b(op_frames_t* frames, const io_t v);
static void op_frames_in_c(op_frames_t* frames, const io_t v);
static void op_frames_in_d(op_frames_t* frames, const io_t v);
static void op_frames_in_step(op_frames_t* frames, const io_t v);
static void op_frames_in_index(op_frames_t* frames, const io_t v);
static void op_frames_in_key(op_frames_t* frames, const io_t v);
static void op_frames_in_ma(op_frames_t* frames, const io_t v);
static void op_frames_in_mb(op_frames_t* frames, const io_t v);
static void op_frames_in_mc(op_frames_t* frames, const io_t v);
static void op_frames_in_md(op_frames_t* frames, const io_t v);

// pickle / unpickle
static u8* op_frames_pickle(op_frames_t* op, u8* dst);
static const u8* op_frames_unpickle(op_frames_t* op, const u8* src);

//-------------------------------------------------
//---- static vars
static const char* op_frames_instring = "INDEX\0  STEP\0   KEY\0    A\0      B\0      C\0      D\0      MUTEA\0  MUTEB\0  MUTEC\0  MUTED\0";
static const char* op_frames_outstring = "A\0      B\0      C\0      D\0      ";
static const char* op_frames_opstring = "FRAMES";

static op_in_fn op_frames_in_fn[11] = {
  (op_in_fn)&op_frames_in_index,
  (op_in_fn)&op_frames_in_step,
  (op_in_fn)&op_frames_in_key,
  (op_in_fn)&op_frames_in_a,
  (op_in_fn)&op_frames_in_b, 
  (op_in_fn)&op_frames_in_c,
  (op_in_fn)&op_frames_in_d,
  (op_in_fn)&op_frames_in_ma,
  (op_in_fn)&op_frames_in_mb, 
  (op_in_fn)&op_frames_in_mc,
  (op_in_fn)&op_frames_in_md
};

//-------------------------------------------------
//---- external func define
void op_frames_init(void* mem) {
  u8 c;
  op_frames_t* frames = (op_frames_t*)mem;
  frames->super.numInputs = 11;
  frames->super.numOutputs = 4;
  frames->outs[0] = -1;

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
  frames->in_val[2] = &(frames->key);
  frames->in_val[3] = &(frames->in[0]);
  frames->in_val[4] = &(frames->in[1]);
  frames->in_val[5] = &(frames->in[2]);
  frames->in_val[6] = &(frames->in[3]);
  frames->in_val[7] = &(frames->mute[0]);
  frames->in_val[8] = &(frames->mute[1]);
  frames->in_val[9] = &(frames->mute[2]);
  frames->in_val[10] = &(frames->mute[3]);

  frames->index = 0;
  frames->step = 0;
  frames->key = 0;
  for (c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->in[c] = 0;
  }
  for (c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->mute[c] = 0;
  }

  // TODO: clear buffer?
}

//-------------------------------------------------
//---- static func define

static void op_frames_in_a(op_frames_t* frames, const io_t v) {
  frames->in[0] = v;
}

static void op_frames_in_b(op_frames_t* frames, const io_t v) {
  frames->in[1] = v;
}

static void op_frames_in_c(op_frames_t* frames, const io_t v) {
  frames->in[2] = v;
}

static void op_frames_in_d(op_frames_t* frames, const io_t v) {
  frames->in[3] = v;
}

static void op_frames_in_ma(op_frames_t* frames, const io_t v) {
  frames->mute[0] = v;
}

static void op_frames_in_mb(op_frames_t* frames, const io_t v) {
  frames->mute[1] = v;
}

static void op_frames_in_mc(op_frames_t* frames, const io_t v) {
  frames->mute[2] = v;
}

static void op_frames_in_md(op_frames_t* frames, const io_t v) {
  frames->mute[3] = v;
}

static void op_frames_in_key(op_frames_t* frames, const io_t v) {
  // store abcd values at index
  u8 i = frames->index % OP_FRAMES_LENGTH;
  for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
    frames->keys[i][c] = frames->in[c];
  }
}			      

static void op_frames_in_step(op_frames_t* frames, const io_t v) {
  // move to next index based on step
  io_t next_idx = (frames->index + v) % OP_FRAMES_LENGTH;
  op_frames_in_index(frames, next_idx);
}

static void op_frames_in_index(op_frames_t* frames, const io_t v) {
  frames->index = frames->index % OP_FRAMES_LENGTH;
  for (u8 c = 0; c < OP_FRAMES_SIZE; c++) {
    net_activate(frames->outs[c], frames->keys[frames->index][c], frames);
  }
}


// pickle / unpickle
u8* op_frames_pickle(op_frames_t* op, u8* dst) {
  // skipping key and step as they are triggers
  dst = pickle_io(op->index, dst);
  dst = pickle_io(op->in[0], dst);
  dst = pickle_io(op->in[1], dst);
  dst = pickle_io(op->in[2], dst);
  dst = pickle_io(op->in[3], dst);
  dst = pickle_io(op->mute[0], dst);
  dst = pickle_io(op->mute[1], dst);
  dst = pickle_io(op->mute[2], dst);
  dst = pickle_io(op->mute[3], dst);

  // TODO: store buffer
  return dst;
}

const u8* op_frames_unpickle(op_frames_t* op, const u8* src) {
  src = unpickle_io(src, &(op->index));
  src = unpickle_io(src, &(op->in[0]));
  src = unpickle_io(src, &(op->in[1]));
  src = unpickle_io(src, &(op->in[2]));
  src = unpickle_io(src, &(op->in[3]));
  src = unpickle_io(src, &(op->mute[0]));
  src = unpickle_io(src, &(op->mute[1]));
  src = unpickle_io(src, &(op->mute[2]));
  src = unpickle_io(src, &(op->mute[3]));

  // TODO: recall buffer
  return src;
}
