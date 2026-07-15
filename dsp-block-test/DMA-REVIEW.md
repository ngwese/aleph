# dsp-block-test DMA / audio review

Several likely problems stand out; the DMA register values themselves look mostly intentional, but a few nearby bugs would prevent correct audio (or make the ping-pong path unsafe under `-O3`).

## High likelihood

### 1. SPORT0 TX `LATFS` does not match the working path

Known-good `bfin_lib` TX setup (`bfin_lib/src/init.c`):

```c
*pSPORT0_TCR1 = TFSR | TCKFE;
*pSPORT0_TCR2 = SLEN_24 | TXSE | TSFSE;
```

`dsp-block-test` (and `bfin_lib_block`) add late frame sync on TX only (`dsp-block-test/serial.c`):

```c
*pSPORT0_TCR1 = TFSR | TCKFE | LATFS;
*pSPORT0_TCR2 = SLEN_24 | TXSE | TSFSE;
```

RX still has no `LARFS`. Early RX + late TX is inconsistent with the production config and with itself. That alone can make the test look “broken” even if DMA IRQs fire.

### 2. Ping-pong buffer pointers are not `volatile`

ISRs update `audioIn` / `audioOut`; main reads them at `-O3`. They are plain globals (`dsp-block-test/audio.h`):

```c
buffer_t *audioIn;
buffer_t *audioOut;
```

The compiler can reuse a stale pointer across blocks and process the buffer DMA is currently filling. `audioRxDone` / `audioTxDone` being volatile does not protect the pointers.

### 3. 24↔32-bit format never handled in descriptor mode

ISRs note it but leave it undone (`dsp-block-test/isr.c`):

```c
#if DMA_DEINTERLEAVE_PINGPONG
  // FIXME: need to convert from 24b to 32b
  if(inBufFlag) {
    audioIn = &inputChannels1;
```

Pass-through of raw RX words can still audibly work (same bit pattern out), but anything generating `fract32` (e.g. `osc_process_block`) will be wrong without `<< 8` / `>> 8` as in `bfin_lib`. Descriptor DMA cannot do that shift for you.

## DMA / ISR logic issues

### 4. Autobuffer path indexes by bytes instead of samples

If `DMA_DEINTERLEAVE_PINGPONG` is turned off (`dsp-block-test/isr.c`):

```c
for(i=0; i<BLOCKSIZE; i++) {
  audioIn[0][i] = audioRxBuf[i * SAMPLESIZE] << 8;
  audioIn[1][i] = audioRxBuf[i * SAMPLESIZE + 1] << 8;
```

`audioRxBuf` is `fract32[]`, so the stride should be `CHANNELS` (4), not `SAMPLESIZE` (4 bytes). Same bug on TX. Interleaved layout would be `[i*CHANNELS + ch]`.

### 5. Descriptor / 2D config looks plausible, with caveats

For the enabled ping-pong path:

| Setting | Value | Notes |
|--------|--------|--------|
| `DMA_FLOW_DESC` | `0x7400` | FLOW=large list + NDSIZE=4 — correct |
| `X_COUNT` / `X_MOD` / `Y_COUNT` / `Y_MOD` | deinterleave into `[ch][frame]` | Formula matches BF 2D “Y replaces last X modify” behavior |
| Circular descriptors | patched in `init_dma` | OK |
| Map `DMA1`→SPORT0 RX, `DMA2`→SPORT0 TX | `0x1000` / `0x2000` | Matches working code |
| `SIC_IMASK` `0x600` | DMA1+DMA2 | OK |
| `SIC_IAR1` `0x33322231` | DMA1→IVG10, DMA2→IVG9 | Matches EVT9/EVT10 handlers |

So the descriptor math and peripheral map are not the obvious smoking gun; SPORT framing and buffer ownership are more suspicious.

### 6. Small related footguns

- `audioIn` / `audioOut` start uninitialized; first use only works because ISRs run first — still fragile.
- `l1_data_A/B` attributes are on the `extern` declarations, not the definitions in `audio.c`, and the linker script never puts `.l1.data.B` in `MEM_L1_DATA_B` (everything lands in bank A). Unlikely to stop DMA alone; bank separation is ineffective.
- Production block path also requires `processAudio` via SPI; this test correctly skips that, so “doesn’t work” here is not that gate.

## Fixes applied

1. Removed `LATFS` from SPORT0 TX (`serial.c`) to match `bfin_lib`.
2. Made `audioIn` / `audioOut` `buffer_t * volatile`, with safe initial values (`audio.h` / `audio.c`).
3. Fixed autobuffer ISR indexing to use `CHANNELS` instead of `SAMPLESIZE` (`isr.c`).
4. Moved `l1_data_*` / `aligned` attributes onto buffer definitions (`audio.c`).
5. Corrected SIC interrupt comment (sport0 TX, not sport1).

Build: `aleph-builder make` in `dsp-block-test/` succeeds (`dsp-block-test.ldr`).

## Still to verify on hardware

Confirm LEDs toggle (ISRs running). If they do but audio is wrong, focus on 24↔32 format / remaining framing; if they don’t, dig into SPORT enable / codec clocks / IRQ clear path.
