# M5 Controller Phase-Consumer Convergence

## Purpose

Audit and repair the complete current controller/device class in which a
consumer treats one ready transition as permission to complete a multi-phase
operation. T510 exposed that defect in the default-PC/AT ATA PIO firmware
path: one DRQ cannot authorize the following sectors after the HDC has
published a new busy/deadline/DRQ transition.

## Scope And Constraints

The finite owner set is PIC, DMA, PIT, RTC, KBC, XT keyboard/PPI, FDC, all
selected HDC personalities, and VADP. Each S traces its one owner through the
actual firmware, DMA, IRQ, Core deadline and VM consumer paths. CPU instruction
execution, synchronous register-only writes, and presentation copies are
included only when they consume one of those owner transitions.

No generic polling framework, VM scheduler, duplicate controller state, or
profile-specific Core branch is permitted. A consumer observes its owner's
existing status/deadline/IRQ contract; it cannot manufacture readiness. The
manual remains primary; existing external emulator code is corroborating only.

## Required Sequence

1. Freeze the cross-owner inventory and the exact bad shape: initial
   ready/DRQ/result observed once, followed by a further owner phase without a
   corresponding fresh observation.
2. Audit PIC with every CPU/interrupt consumer and repair any proven stale
   acknowledgement or re-publication path.
3. Audit DMA with every request/provider/terminal-count consumer, including
   FDC and Xebec downstream use.
4. Audit PIT, then RTC, with their IRQ and firmware consumers.
5. Audit KBC, then XT keyboard/PPI, with command, serial/BAT, IRQ and
   firmware consumers.
6. Audit FDC including DMA and non-DMA phases, result consumption and firmware
   command paths.
7. Re-audit all HDC personalities and their PIO/DMA/firmware consumers against
   the T510 multi-sector rule.
8. Audit VADP display/raster/attribute transitions and copied presentation
   consumers; retain its sole state/snapshot owner.
9. Run the full unit suite after every S. Close only after the complete unit
   suite, external-ROM/disk integration suite, and a rebuilt stripped 0511
   artifact pass.

## Exit Criteria

Every owner/consumer row has a disposition: fixed, not applicable because the
same owner consumes the repeated phase, or an explicit lower-tier/unsupported
boundary with its admission path. Every proven defect is repaired at the
consumer or owner that owns the missing observation, with a regression that
crosses at least two phases. No similar production hit remains undocumented.
