# M5 T574 S1 — PC110 C-Segment Trace

## Scope

This record tests whether the approved TFT firmware selects a C-segment video
ROM map before the first post-reset failure.  It does not qualify POST, Setup,
DOS, an EXE, or a guessed VL82C420 mapping.

## Inputs and source grades

- The external TFT firmware is the approved 256 KiB asset at
  `profiles-nxvm/ibm-pc110-tft`; its four 64 KiB banks have the observed
  headers `55 aa 00 eb` (bank 0) and `55 aa 47 eb` (bank 2).  This is L2 asset
  observation, not an in-repository ROM copy.
- Open-Source-PC110's live-dump record is L2 evidence that the running machine
  has a writable F-segment shadow and a C000 C&T video BIOS after POST.  It
  also corrects an older claim: `EC/ED` is the power MCU, not VL82C420.
- PC110-EMU and PC110-QEMU are L2 behavior references only.  Their static
  video-bank mappings and POST completion paths are not implementation input.

## Method

`mypc110-real-firmware-post-diagnostic` runs the normal Core/X86-NXVM executor
for eight million instructions, records every successful access to `22/24/25`,
`74/76`, `EC/ED`, `60/61/64`, and observes C000 and the system BIOS `INT 10h`
site.  No host handler, IVT/BDA modification, firmware-address dispatch or
second executor is involved.

## Results

Both release widths produced the same material facts:

```text
executed=8000018 resets=1 warm=0000 c000-header=55aa00eb
first exception=20000000 at F000:6044
post-reset exception=20000000 at 0040:6044
C000 entries=0/0; F000:4CA9 INT 10h entries=0/1
recorded candidate-port accesses=0
```

The firmware reaches a CPU reset/fault boundary before it emits a captured
SCAMP, block-2, power-MCU, KBC, or port-61 transaction and before it transfers
to C000.  Therefore the required reset/default C-segment selection and its
transition protocol are not presently observable.  The source gate in the
candidate is not met.

## Retained implementation

The App owns a mutable 64 KiB F-segment shadow initialized from raw Flash bank
3 on each machine reset and overlays it at `F0000`/`FFFF0000`.  This preserves
the independently observed firmware layout without inventing a C-segment
decoder.  C/D/E continue to use the immutable raw Flash fallback solely for
the existing normal reset fetch; no C000 transfer is claimed.  The associated
unit proves shadow writes and reset restoration.

## Disposition

T574 closes as a source-gate receiver, not a completed C-segment contract.
The next receiver must first reproduce the protected bootstrap failure or
obtain a complete, lawful trace/manual for the exact mapping sequence.  It
must not add a static video alias, warm-signature write, PC/AT fallback or
synthetic BIOS service.
