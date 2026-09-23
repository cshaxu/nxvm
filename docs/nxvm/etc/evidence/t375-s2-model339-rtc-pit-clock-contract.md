# T375 S2: Model-339 RTC And PIT Clock Contract

## Decision

The Model-339 descriptor, not the generic PC/AT descriptor and not a device
side path, now supplies the two source-backed clock conversions selected by
T375 S1:

| Consumer | IBM 6280099 fact | Profile conversion from nominal 8 MHz source | Reset phase |
| --- | --- | --- | --- |
| 8254 PIT | System Board 1-22 labels each `CLKIN` as `1.193182 MHz OSC`. | `1193182 / 8000000 = 596591 / 4000000` | zero |
| MC146818 RTC | System Board 1-57 says divider setting `010` selects the `32.768-kHz time base`. | `32768 / 8000000 = 64 / 15625`; one RTC second is 32768 delivered RTC ticks. | zero |

The primary source is the [IBM Personal Computer AT Technical Reference,
March 1986, 6280099](https://www.minuszerodegrees.net/manuals/IBM/IBM_5170_Technical_Reference_MAR86.pdf),
System Board pages 1-22 and 1-57.  The PIT scalar preserves the manual's
printed 1.193182-MHz precision; it does not assert an unrecorded crystal
tolerance or a more precise board frequency.

`pc_at_profile.c` owns these two values in the selected Model-339 descriptor.
The descriptor's existing CPU contract remains the sole path into the core
clock plan, and session composition remains the sole path that supplies the
RTC second divisor.  Default PC/AT retains its prior generic values, so this
does not turn a non-Model-339 profile into an IBM board claim.

## Proof

`vm-model-339-clock-contract-smoke` proves all of the following without host
time, a ROM, or guest media:

- Model-339 and default PC/AT expose their distinct descriptor values;
- session composition copied the two ratios into the actual core clock domains
  and copied 32768 into the configured RTC;
- 4,000,000 nominal source ticks convert to 596,591 PIT ticks and 15,625 to
  64 RTC ticks; and
- a core reset restores the same zero-phase conversion.

The focused local replay passed:

```text
M5:T208:S2:DEFAULT-PC-AT-PROFILE:OK
M5:T366:S5:MODEL339-COMPOSITION:OK
M5:T375:S2:MODEL339-CLOCK-CONTRACT:OK
M5:T256:S3:RATIONAL-CLOCK:OK
```

## Boundary And Transfer

These ratios convert successful-retirement `elapsed_ticks`, whose existing
owner is `machine.c`; they do not make those ticks wall time or prove device
progress while the CPU is halted or externally unavailable.  They also do not
add a memory/ROM/ISA READY value, DMA rate or grant phase, PIC/INTA latency,
FDC rotational or command time, 8042 serial time, CGA raster cadence, reset
settling time, or electrical waveform.  Those Model-339 board/phase questions
remain with T375 after this S.

No ROM, media, binary, external source code, or reference-runtime behavior was
imported.  86Box, MAME, and PCjs were not used as a source for either scalar.
