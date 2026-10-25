# M5 T580 S33: PC110 outer-delay return

## Question

S32 observed the real `F000:EA5Bh` outer-delay tail with `BL=6Fh`, but its
wall-time extrapolation included prior BIOS startup. Does the real `EA38h`
outer delay complete, and where does its `EA7Fh` return land?

## Method

The existing post-C000 diagnostic replaces its former caller mode with one
outer-return mode. It observes only already executed retirement points: entry
at `F000:EA38h`, each `F000:EA55h` loop tail, `F000:EA7Fh`, and the first
returned-to instruction. The 13,000,000-instruction limit remains containment
only; the return facts are the success condition. No guest state, timing
source, firmware byte, device route, or production source changes.

## Observation

The external, hash-pinned TFT BIOS produced the same result on both host
widths:

```text
PC110:POST:c000-port61-outer=entered:1 initial-bl=70 iterations=112 returned:1 caller=f000:000053dd bytes=b9 33 c2 e8
```

The final named CTest completed in 40.01 seconds on x64 and 45.73 seconds on
x86.
The `F000:53DDh` caller is the static continuation after `call EA38h`; its
leading bytes decode as `mov cx,0C233h; call EA80h`. This is continued BIOS
delay work through the existing port-61 service, not a missing device receiver.

## Disposition

The former caller-only mode is removed, so there is one semantic checkpoint
rather than overlapping slow diagnostics. S32's hour-scale inference is
superseded: it was based on startup-inclusive timing, not a measured loop.
This result does not prove POST completion, setup, storage, panel operation,
DOS boot, or publishable MyPC110 artifacts. It selects no production change.
