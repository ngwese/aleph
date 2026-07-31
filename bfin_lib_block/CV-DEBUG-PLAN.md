# cv output debug plan (block library)

status: `dacs_block` produces no CV movement. the frame module `modules/dacs`
works on the same hardware (all four channels), with an occasional glitch to
0 V on the last channel during sweeps.

this document compares the two drivers register by register, states what the
frame path actually does on the wire, lists the defects found in the block
path (ranked by confidence), and gives an ordered plan of things to try.

related files:

- frame: [`bfin_lib/src/cv.c`](../bfin_lib/src/cv.c),
  [`bfin_lib/src/init.c`](../bfin_lib/src/init.c) (`init_sport1`, `init_DMA`)
- block: [`src/cv.c`](./src/cv.c), [`src/serial.c`](./src/serial.c)
  (`init_sport1`), [`src/main.c`](./src/main.c)
- module: [`modules_block/dacs_block/dacs_block_module.c`](../modules_block/dacs_block/dacs_block_module.c)
- spec: [`SPEC_CV.md`](./SPEC_CV.md)

---

## side-by-side

| item | frame (`bfin_lib`) | block (`bfin_lib_block`) |
|------|--------------------|--------------------------|
| `SPORT1_TCR1` | `ITCLK \| ITFS \| TFSR` | same |
| `SPORT1_TCR2` | `24 \| TXSE` (SLEN+1 = **25** bits) | `SLEN_24 \| TXSE` (**24** bits) |
| `SPORT1_TCLKDIV` | 1 (~27 MHz) | same |
| `SPORT1_TFSDIV` | not written (0) | **24** (25-clock frame period) |
| word packing | `buf << 1` (pad bit) | `buf` (no pad) |
| DMA4 config | `WDSIZE_32 \| FLOW_1` | `\| DMA2D` as well |
| DMA4 `X_COUNT` / `X_MODIFY` | **1** / 4 | **2** / **0** (was 4 / 4) |
| DMA4 `Y_COUNT` / `Y_MODIFY` | unused | **4** / **4** |
| buffer | `volatile u32 cvTxBuf` (one word) | `volatile u32 cvTxWords[4]` |
| update | `cv_update(ch, v)` per audio frame, round robin | `cv_set` ×4 + `cv_commit` per block |
| `SIC_IMASK` DMA4 bit (12) | set (`0x3200`) | clear (`0x2600`) |

everything else that matters (PLL 108 MHz, `FIO_DIR` for reset/LDAC, DAC reset
pulse, DMA4 peripheral map `0x4000`, no data cache) is identical.

---

## how the frame path actually works on the wire

this matters because the block driver was written as if the SPORT consumed one
memory word per DAC frame. it does not.

1. **`TXSE` means two words per frame.** SPORT1 transmits primary (`DT1PRI`)
   and secondary (`DT1SEC`) *simultaneously* inside one frame sync. a single
   DMA channel (DMA4) feeds both, fetching **two words per frame**, primary
   first, then secondary. only `DT1SEC` is wired to the AD5686.
2. the frame lib sidesteps this by setting `X_COUNT = 1`: the same word is
   fetched for both halves, so the secondary always carries the intended
   command. the comment in `init_DMA` says exactly this:

```159:161:bfin_lib/src/init.c
  // DMA inner loop count
  /// primary tx data is dummy!
  *pDMA4_X_COUNT = 1;
```

   older abandoned code in the sim tree spells the interleave out even more
   plainly — a two-word buffer with a zero in the primary slot:

```65:66:utils/bfin_sim/src/cv.c
/*   cvTxBuf[0] = 0x00000000; */
/*   cvTxBuf[1] = buf; */
```

3. **the 25-bit "kludge" is a SYNC generator.** with `LTFS = 0` the frame sync
   is active high, and the AD5686 `/SYNC` is driven from it, so SYNC is *low*
   during the frame body and *high* during the FS pulse. with early frame sync
   (`LATFS = 0`) the FS for the next frame is asserted during the last clock of
   the current one. so a 25-bit word gives: 24 data bits with SYNC low, then
   one clock with SYNC high (the pad bit is clocked and ignored). that rising
   edge is what latches the AD5686, and it also supplies the ≥20 ns SYNC-high
   time the part requires between writes.

the current block config (`SLEN_24` + `TFSDIV = 24`) is intended to produce the
same waveform by a cleaner route: frame period 25 clocks, 24 data bits, one
idle clock in which FS is asserted. **this is believed equivalent but has not
been confirmed on hardware or a scope.** see finding 3.

---

## findings

### 1. the four-word buffer only delivers half the channels — critical

`cvTxWords[4]` with `X_COUNT = 4` is two frames, not four:

| DMA index | destination | content |
|-----------|-------------|---------|
| 0 | `DT1PRI` (unconnected) | ch0 command |
| 1 | `DT1SEC` (DAC) | ch1 command |
| 2 | `DT1PRI` (unconnected) | ch2 command |
| 3 | `DT1SEC` (DAC) | ch3 command |

so channels 0 and 2 are thrown away on the unused pin, and only 1 and 3 ever
reach the part (or 0 and 2, if the fetch phase happens to be inverted). this
is true of both the stop-mode version and the current autobuffer version — the
buffer layout was wrong in both.

confidence: high. it follows directly from `TXSE` plus the two comments quoted
above. it is also the cheapest thing to falsify: sweep one channel at a time
and see whether exactly two of them respond.

`TXSE` cannot be dropped: the DAC is on `DT1SEC`, and there is no bit to
disable the primary transmitter (`TSPEN` enables it implicitly). so each
channel word has to be sent down both halves of the frame.

**fix (applied):** 2D autobuffer that reads each word twice out of the existing
four-word buffer, in `init_dma_cv()`:

```c
#define CV_DMA_CONFIG (WDSIZE_32 | DMA2D | CV_DMA_FLOW_1)

*pDMA4_X_COUNT = 2;            /* same word to primary and secondary */
*pDMA4_X_MODIFY = 0;
*pDMA4_Y_COUNT = CV_CHANNELS;  /* advance one word per frame */
*pDMA4_Y_MODIFY = 4;
```

blackfin applies `Y_MODIFY` instead of `X_MODIFY` on the last inner iteration,
so the fetch order is `c0 c0 c1 c1 c2 c2 c3 c3`. sending each word twice also
makes this immune to which half of the frame the DMA starts on — unlike a
layout with no-ops in the primary slots, which would deliver nothing but
no-ops if the phase were inverted.

an equivalent alternative is an eight-word buffer with each channel word
duplicated in memory and a plain 1D `X_COUNT = 8`, `X_MODIFY = 4`. same
waveform, same eight fetches per pass; it just moves the redundancy from the
DMA registers into the buffer.

### 2. nothing proves `module_process_block` is running — high

`cv_commit()` is only reached from the module, and the block main loop gates on
a flag the AVR32 must set:

```59:59:bfin_lib_block/src/main.c
    if(audioTxDone && audioRxDone && processAudio) {
```

`processAudio` starts at 0 and is only set by `MSG_ENABLE_AUDIO` over SPI.
`dacs_block` writes silence to all four audio outs, so **there is no observable
difference between "CV is broken" and "the module never runs"**. the frame
`dacs` module has the same gate but a different call path (audio RX ISR), so
its working state does not prove the block loop is being entered.

this must be settled before interpreting any other result.

### 3. the 24-bit / `TFSDIV` framing change is unverified — medium

commit `bbd1c7ff` moved the block path from the frame lib's 25-bit encoding to
`SLEN_24` plus `TFSDIV = 24`. the reasoning is in the section above and the
register constraint is satisfied (`TFSDIV` 24 ≥ `SLEN` 23, giving exactly one
idle clock ≈ 37 ns at 27 MHz), but it has never been on a scope. risks:

- if `TFSDIV` interacts badly with data-dependent frame sync (`DITFS = 0`) the
  frame rate or the FS position could differ from what is assumed here.
- if the DAC's SYNC is not simply `TFS1` (external logic, inversion), the
  analysis above is wrong for both libraries.

this change and the buffer-layout change are independent; do not evaluate them
at the same time (see the plan).

### 4. stop-mode restarts reprogram a possibly running channel — medium

historical, but worth recording so it is not reintroduced. the pre-`b6f239e4`
`cv_commit()` wrote `DMA4_START_ADDR`, `X_COUNT` and `X_MODIFY` and then set
`DMAEN` on every commit. writing DMA registers other than `CONFIG` while the
channel may still be active is not allowed by the HRM and can wedge the
channel; it also lets the primary/secondary fetch phase drift between bursts,
so which channels reach the DAC could change over time. the 2017 frame-lib
experiment `ecf1b324` ("fix cv output by disabling DMA4 autobuffer") did the
same thing and was reverted in `11e50bc4`.

keep the continuous autobuffer.

### 5. DMA4 interrupt is masked in the block build — low

frame sets `SIC_IMASK = 0x3200` (bits 9, 12, 13 — DMA1, **DMA4**, DMA5); block
sets `0x2600` (bits 9, 10, 13 — DMA1, DMA2, DMA5), so the DMA4 bit is clear.
neither library sets `DI_EN` on DMA4, so no interrupt should be generated
either way and this should not matter. it is listed only because the frame lib
carries a suspicious comment ("i don't know why we need to unmask IVG12 and
IVG13 ... if we leave the flags zeroed nothing ever happens"), and flipping one
bit is a two-minute experiment.

### 6. `cvTxWords` is written while DMA reads it — low

each `u32` store is atomic, so no word can tear, but a burst can straddle a
commit and mix old and new channel values for one pass. at CV rates this is
inaudible. only worth addressing (double buffer + `START_ADDR` swap between
bursts) if a real artifact is observed.

### 7. `cv_pack` assumes a non-negative `fract32` — low

`(val >> 15) & 0xffff` on a negative value yields 0xffff (full scale), not 0.
both libraries share this. if any slew or param path can transiently go
negative or overshoot past `0x7fffffff`, the output jumps to full scale rather
than clipping. clamp if the frame glitch investigation points here.

### 8. why the frame path's last channel can drop to 0 V — hypothesis

the 25-bit encoding depends on the DAC latching the **first** 24 bits and
ignoring the 25th. if SYNC ever slips by one clock, the part instead keeps the
**last** 24 bits, which shifts the command field by one:

- the transmitted stream is `buf[23..0]` followed by a `0` pad
- a one-clock slip makes the received command nibble `0b011a3`, where `a3` is
  the top address bit — set only for channel 3

so a slip on channels 0–2 decodes as command `0110` (software reset) and on
channel 3 as `0111` (internal reference setup). either one drives outputs to
0 V, and the channel-3 case is a distinct code path — which fits "the final
channel occasionally drops to 0 V" better than a generic wiring fault. this is
unproven; the alternative explanations are a slew/latch bug in the module (cf.
`e9b6494a`, "waves: fix CV slew latch bug"), finding 7, or analog hardware (cf.
`6d7e9e06`, "might be hardware after all").

if this hypothesis holds, the 24-bit + `TFSDIV` framing is not just cosmetic —
it removes the ambiguity that causes the glitch, and should eventually be
backported to `bfin_lib`.

---

## plan, in priority order

each step is written so that it either fixes the problem or produces
information. do exactly one change per hardware test.

### step 0 — prove the module runs (blocking prerequisite)

do not debug the DAC until this is answered.

- toggle `LED3` every N calls of `cv_commit()` (frame lib used the same trick:
  a counter in `cv_update` toggling `LED4`), or
- temporarily make `dacs_block` emit a quiet tone on audio out 0 instead of
  silence, or
- read the xrun/meter SPI counters from bees to confirm blocks are processing.

if the LED does not toggle: the problem is `processAudio` / SPI enable / scene
recall, not the DAC. `d69ec5fc` records a historical "SPI interrupt problem
when initialized from scene recall" for the block library, so this is a real
possibility.

### step 1 — test the 2D DMA fix, one channel at a time

the 2D autobuffer from finding 1 is applied in `init_dma_cv()`. sweep each CV
param **individually** from bees and record which outputs move.

| observation | conclusion |
|-------------|-----------|
| all four channels track their params | finding 1 was the fault; go to step 7 |
| exactly two channels move | 2D setup is wrong; check `Y_COUNT` / `X_MODIFY = 0` took effect |
| no channel moves | framing or wire format is also wrong; go to step 3 |

expected on success: all four channels update once per block, ≈3 kHz for the
whole set at blocksize 16.

### step 2 — sanity-check the DMA registers if the result is ambiguous

`X_MODIFY = 0` and 2D autobuffer are both unusual enough to be worth
confirming before drawing conclusions from step 1. read `DMA4_CURR_ADDR` and
`DMA4_CURR_X_COUNT` / `DMA4_CURR_Y_COUNT` back over the SPI debug path (or
halt in a debugger) and check the address only advances every second fetch and
wraps after four.

as a fallback with simpler semantics, swap to the eight-word duplicated buffer
described in finding 1 — plain 1D, `X_COUNT = 8`, `X_MODIFY = 4`. if that works
where 2D does not, the 2D configuration is the problem, not the interleave
theory.

### step 3 — if nothing moves, bisect the framing change

restore the exact frame-lib wire format while keeping the fixed buffer:

- `src/serial.c`: `*pSPORT1_TCR2 = 24 | TXSE;` and remove the `TFSDIV` write
- `src/cv.c`: `return buf << 1;` in `cv_pack`

this isolates the buffer fix from the 24-bit framing change. if CV starts
working, finding 3 is the culprit and the framing change needs scope time
before being reintroduced. if it still does not work, the fault is upstream of
the wire format (step 0 or step 4).

### step 4 — frame-lib-equivalent fallback

if steps 1–3 all fail, reduce the block driver to something bit-identical to
the known-good frame driver and give up multi-word bursts for now:

- one `volatile u32 cvTxWord`, `X_COUNT = 1`, continuous autobuffer
- `cv_commit()` writes **one** channel per call, round robin, exactly like
  `module_process_frame` in `modules/dacs`
- call `cv_commit()` four times per block (or once per block per channel, which
  gives 750 Hz per channel at blocksize 16 — adequate for panel CV)

if this works and step 1 does not, the difference is purely in multi-word DMA
behaviour, which narrows the search enormously.

### step 5 — cheap register experiments

only if the above is inconclusive:

- set the DMA4 bit in the block `SIC_IMASK` (`0x2600` → `0x3600`) to match the
  frame lib (finding 5)
- try `DITFS = 1` (data-independent frame sync) so FS is free-running at the
  `TFSDIV` rate rather than gated on data availability
- drop `TCLKDIV` to a slower clock (e.g. 5 → ~9 MHz) to rule out setup/hold
  marginality on the SYNC edge

### step 6 — scope / logic analyzer confirmation

if hardware access allows, capture `TFS1`, `TSCLK1`, `DT1SEC` and confirm:

- frame period is 25 `TSCLK1` cycles, SYNC high for exactly one of them
- exactly 24 falling edges occur with SYNC low
- four *distinct* commands appear per buffer pass, with the expected one-hot
  address in bits 19:16
- the command nibble reads `0011` (write-and-update), not `0110` / `0111`
  (which would confirm finding 8)

also worth a schematic check: confirm `/SYNC` is driven from `TFS1` directly
(no inverter or latch) and that `DT1PRI` is genuinely unconnected.

### step 7 — follow-ups once CV works

- backport the 24-bit + `TFSDIV` framing to `bfin_lib` and re-test the channel
  3 glitch (finding 8)
- clamp `cv_pack` input to `[0, 0x7fffffff]` (finding 7)
- update [`SPEC_CV.md`](./SPEC_CV.md): the DMA model section still describes a
  four-word buffer and still shows the 25-bit packing snippet
- consider double buffering only if a commit-straddling artifact is observed
  (finding 6)

---

## summary

| # | finding | confidence | effort |
|---|---------|-----------|--------|
| 1 | four-word buffer feeds half the words to the unused primary pin | high | fixed, untested |
| 2 | no evidence the block module runs at all (silent module + `processAudio` gate) | high | small |
| 3 | 24-bit + `TFSDIV` framing never verified on hardware | medium | small |
| 4 | stop-mode restarts reprogrammed a live DMA channel (already removed) | medium | done |
| 5 | DMA4 bit masked in block `SIC_IMASK` | low | trivial |
| 6 | commit races the running autobuffer | low | medium |
| 7 | `cv_pack` maps negative input to full scale | low | trivial |
| 8 | 25-bit encoding can decode as reset / ref-setup on a one-clock slip | hypothesis | n/a |

the single most likely reason `dacs_block` shows no CV is finding 1, but
finding 2 must be ruled out first or the test result cannot be trusted.
