# IBM PC/AT 5170 Board And Device Phase-Timing Closure

## Purpose

Close the Model-339 board-local availability, wait, service, and event-phase
contracts only after its selected devices are functionally complete. This
candidate consumes the accepted 80286, capability-ledger, and selected-device
results and precedes the fresh final 5170 L3 audit.

## Required scope

For the frozen 8 MHz Type-3/Rev.3/512 KB/CGA/no-fixed-disk configuration, map
CPU fetch/memory/I/O phases, DMA/HOLD, PIC/PIT/RTC, 8042/keyboard, FDC/floppy,
CGA, NMI, reset, and cross-device ordering to source-labelled board-local
availability and service contracts. Use primary IBM/Intel/component material
and project-owned probes. When primary documentation supplies only a range or
none, named same-profile 86Box, MAME, or PCjs behavior may cross-check a
bounded measurement contract; it never becomes a board-fact authority.

## Non-goals and stop conditions

No functional repair hidden in timing work, generic AT value, factory claim for
the aftermarket 1.44 MB drive, fixed-disk route, physical waveform claim,
host-time coupling, or final L3 decision. Transfer an unresolved functional
defect to its earliest owner and a source-exhausted timing boundary visibly to
the final audit.

## Evidence standard

Require a selected-board timing ledger, contention and reset replay,
command-to-IRQ/DRQ/frame traces, exact versus range/reference-exhausted
dispositions, focused regressions and current-gate evidence, plus a complete
handoff to the final 5170 audit.
