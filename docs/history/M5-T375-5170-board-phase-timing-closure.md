# M5 T375: IBM PC/AT 5170 Board And Device Phase-Timing Closure

## Task Record

T375 applies the selected Model-339 8-MHz Type-3/Rev.3/512-KB/CGA/no-fixed-
disk baseline to source-labelled board/device service and phase work. It
follows the selected-device functional closure and precedes the raw-IMG
fidelity candidate and final 5170 Model-L3 audit. It never claims a physical
cycle-exact board model or Model-339 L3 readiness.

## Accepted Work

T375 establishes reset-safe virtual-time publication, selected RTC/PIT and
CGA clock contracts, logical CRTC/status behavior, reference-bounded CGA
cadence, PIT/PIC ordering, named aftermarket-drive service inputs, 500-kbit/s
DMA and non-DMA byte availability, nominal seek cadence, and IBM typematic
defaults plus live F3 decoding. Its S25 ledger verifies that CPU, DMA and PIC
retain one deterministic logical ownership graph.

The detailed sequence and proof are indexed by the [T375 closure audit](../etc/evidence/t375-s26-task-closure-transfer-audit.md).

## Task Closure

S26 is accepted at `3b6af9d6`. T375 closes with the following transfers:

- the queued **8272A raw-IMG sidecar fidelity closure** owns Deleted Data,
  Control Mark and Scan behavior and must replay the accepted FDC timing;
- the queued **IBM PC/AT 5170 final model-L3 audit** owns the visible
  ready/not-ready decision and every source-exhausted Model-339 physical-board
  phase boundary; and
- unselected MFM/ST-506 and I/O-check-NMI routes remain TODO debt.

**IBM PC/AT 5170 Model 339 L3 is not accepted by T375.**
