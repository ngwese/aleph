#ifndef RICKS_TRICKS_H
#define RICKS_TRICKS_H

#include "types.h"

#if ARCH_BFIN
#include "fix.h"
#include "fract_math.h"
#endif


#define TWOPI 6
#define PI 3
static inline void mix_aux_mono(fract32 in_mono, fract32* out_left, fract32* out_right, fract32 left_value, fract32 right_value) {
    *out_right = add_fr1x32(*out_right,mult_fr1x32x32(in_mono, right_value));
    *out_left = add_fr1x32(*out_left,mult_fr1x32x32(in_mono, left_value));
}

static inline void mix_panned_mono(fract32 in_mono, fract32* out_left, fract32* out_right, fract32 pan, fract32 fader) {
    fract32 pan_factor, post_fader;

    pan_factor = (fract32) ( pan );
    post_fader = mult_fr1x32x32(pan_factor, fader);
    *out_left = add_fr1x32(*out_left, mult_fr1x32x32(in_mono, post_fader));

    pan_factor = (fract32) ( FR32_MAX - pan );
    post_fader = mult_fr1x32x32(pan_factor, fader);
    *out_right = add_fr1x32(*out_right, mult_fr1x32x32(in_mono, post_fader));

}

typedef struct {
  fract32 lastIn;
  fract32 lastOut;
} hpf;

typedef struct {
  fract32 lastOut;
} lpf;

typedef struct {
  lpf lpf;
  hpf hpf;
} bpf;

//Takes a fraction in 16_16 radix, returns it in 0_32
static inline fract32 one_over_x_16_16 (fract32 x_16_16) {
  return shl_fr1x32(FR32_MAX / x_16_16, 16);
}

#define TWO_PI_16_16 411775
static inline fract32 hpf_freq_calc (fract32 freq) {
  // 1.0 / (2.0 * pi * dt * fc + 1.0)
  return one_over_x_16_16(add_fr1x32(mult_fr1x32x32(TWO_PI_16_16, freq),
				     1 << 16));
}
static inline fract32 lpf_freq_calc (fract32 freq) {
  // 1.0 / ((1 / 2.0 * pi * dt * fc) + 1.0)
  fract32 temp = mult_fr1x32x32(TWO_PI_16_16, freq);
  return ((temp << 12) / ((1 << 16) + temp)) << 19;
}

fract32 hpf_next_dynamic_precise (hpf *myHpf, fract32 in, fract32 freq);
void hpf_init (hpf *myHpf);
fract32 hpf_next_dynamic (hpf *myHpf, fract32 in, fract32 freq);
void lpf_init (lpf *myLpf);
fract32 lpf_next_dynamic (lpf *myLpf, fract32 in, fract32 freq);
fract32 lpf_next_dynamic_precise (lpf *myLpf, fract32 in, fract32 freq);

void bpf_init (bpf *myBpf);
fract32 bpf_next_dynamic_precise (bpf *myBpf, fract32 in, fract32 hpf_freq, fract32 lpf_freq) ;

typedef struct {
  s32 phase;
  fract32 freq;
} phasor ;

typedef struct {
  s32 cosPhase;
  s32 sinPhase;
} quadraturePhasor ;

fract32 lpf_next_dynamic (lpf *myLpf, fract32 in, fract32 freq);
void phasor_init (phasor *phasor);
s32 phasor_next_dynamic (phasor *phasor, fract32 freq);
s32 phasor_next (phasor *phasor);
s32 phasor_read (phasor *phasor, s32 freq);
s32 phasor_pos_next_dynamic (phasor *phasor, fract32 freq);
s32 phasor_pos_read(phasor *phasor);
void quadraturePhasor_init (quadraturePhasor *phasor);
void quadraturePhasor_pos_next_dynamic (quadraturePhasor *phasor, fract32 freq);
s32 quadraturePhasor_pos_sinRead(quadraturePhasor *phasor);
s32 quadraturePhasor_pos_cosRead(quadraturePhasor *phasor);
s32 quadraturePhasor_sinRead(quadraturePhasor *phasor);
s32 quadraturePhasor_cosRead(quadraturePhasor *phasor);
fract32 s32_flatTop_env (s32 pos, s32 fadeRatio);
fract32 osc (fract32 phase);
fract32 osc_triangle (fract32 phase);
fract32 osc_square (fract32 phase);

#define simple_slew(x, y, slew)					\
  (x = add_fr1x32( (fract32) (y),				\
		   mult_fr1x32x32(sub_fr1x32(FR32_MAX,		\
					     (fract32) (slew)),	\
				  sub_fr1x32((fract32) (x),	\
					     (fract32) (y)))))

#define simple_slew_cheat(x, y, slew)					\
  (x = add_fr1x32( (fract32) (y),					\
		   mult_fr1x32x32_cheat(sub_fr1x32(FR32_MAX,		\
						   (fract32) (slew)),	\
					sub_fr1x32((fract32) (x),	\
						   (fract32) (y)))))


#define SLEW_1MS (TWOPI * hzToDimensionless(1000))
#define SLEW_10MS (TWOPI * hzToDimensionless(10))
#define SLEW_100MS (TWOPI * hzToDimensionless(10))
#define SLEW_1S (TWOPI * hzToDimensionless(1))
#define SLEW_4S  (1 << 16)

#define simple_lpf(x, y, hz) \
  simple_slew(x, y, TWOPI * hzToDimensionless(hz))

#define SR 48000
#define hzToDimensionless(hz) ((fract32)((fract32)hz * (FR32_MAX / SR)))

#define fadeOut(x) sub_fr1x32(FR32_MAX, mult_fr1x32x32( x, x))
#define fadeIn(x) fadeOut(sub_fr1x32(FR32_MAX, x))

#define PITCH_DETECTOR_RADIX_INTERNAL 4
#define PITCH_DETECTOR_RADIX_EXTERNAL 8
#define PITCH_DETECTOR_RADIX_TOTAL (PITCH_DETECTOR_RADIX_INTERNAL + PITCH_DETECTOR_RADIX_EXTERNAL)

typedef struct {
  fract32 currentPeriod;
  fract32 lastIn;
  fract32 period;
  fract32 phase;
  s32 nsamples;
  s32 nFrames;
  hpf dcBlocker;
  lpf adaptiveFilter;
} pitchDetector;

void pitchDetector_init (pitchDetector *p);
fract32 pitchTrack (pitchDetector *p, fract32 preIn);
fract32 pitchTrackOsc (pitchDetector *p);
fract32 s32_halfWave_env (fract32 pos) ;
float interp_bspline_float (float x, float _y, float y, float y_, float y__);
fract32 interp_bspline_fract32 (fract32 x, fract32 _y, fract32 y, fract32 y_, fract32 y__);


typedef struct {
  unsigned short radix;
  fract32 remainder;
  u32 slewRate;
} radixLinSlew;

static const fract32 LINSLEW_1MS = FR32_MAX / 48;
static const fract32 LINSLEW_10MS = FR32_MAX / 48 / 10;
static const fract32 LINSLEW_100MS = FR32_MAX / 48 / 100;
static const fract32 LINSLEW_1S = FR32_MAX / 48 / 1000;
static const fract32 LINSLEW_10S = FR32_MAX / 48 / 10000;

typedef struct {
  fract32 up;
  fract32 down;
} asymLinSlew;

typedef struct {
  unsigned short radix;
  fract32 remainder;
  fract32 speed;
} logSlew;

typedef struct {
  fract32 val;
  asymLinSlew slew;
} trackingEnvelopeLin;

typedef struct {
  fract32 val;
  fract32 up;
  fract32 down;
} trackingEnvelopeLog;

void radixLinSlew_init (radixLinSlew* slew, unsigned short radix);
void radixLinSlew_next (fract32* current, fract32 target, radixLinSlew *slew);
void linSlew_next (fract32* current, fract32 target, fract32 slewRate);
void asymLinSlew_init (asymLinSlew* slew);
void asymLinSlew_next (fract32* current, fract32 target, asymLinSlew* slew);
void radixLogSlew_init (logSlew *slew, unsigned short radix);
void radixLogSlew_next (fract32* current, fract32 target, logSlew* slew);
void fine_logSlew(fract32* current, fract32 target, fract32 speed);
void coarse_logSlew(fract32* current, fract32 target, fract32 speed);
void normalised_logSlew(fract32* current, fract32 target, fract32 speed);
void trackingEnvelopeLin_init (trackingEnvelopeLin* env);
fract32 trackingEnvelopeLin_next (trackingEnvelopeLin* env, fract32 in);
void trackingEnvelopeLog_init (trackingEnvelopeLog* env);
fract32 trackingEnvelopeLog_next (trackingEnvelopeLog* env, fract32 in);



#endif
