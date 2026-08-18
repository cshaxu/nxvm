# M5 T427: DeskPro CPU BUSRDY Gate

## Outcome

T427 S1 adds one original-source, Core-owned CPU BUSRDY completion gate to the
existing chargeable external CPU-memory-cycle wait and retirement path. Model-40
selects it; IBM 5170 Model 339 does not. Cold reset restores ready. The accepted
implementation is `d1887160`.

## Evidence And Verification

The retained [S1 evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md)
records the source tier, ownership, focused regressions, full serial
current-gate, documentation governance and diff-hygiene result.

## Closure And Transfer

This closes only the bounded discrete CPU BUSRDY receiver. It does not close an
asynchronous CPU external-cycle producer, BWAIT waveform, peripheral binding,
D4 PAL phase model, calibrated clock duration, or physical/L3 acceptance. The
remaining receiver stays in the DeskPro physical-cycle proposal and Queue.