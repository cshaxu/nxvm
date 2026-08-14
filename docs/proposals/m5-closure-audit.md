# M5 Closure Audit

## Purpose

Reconcile current source, product boundaries, CMake/test coverage, accepted
evidence, and open debt before any M5-close decision.  This candidate is an
audit and planning package, not permission to call M5, Windows compatibility,
or physical L3 complete.

## Required audit surface

The audit must cover every current core-machine owner and PC/AT composition
route: CPU-visible memory/port transactions; PIC, PIT, RTC/CMOS, dual DMA,
FDC, ATA, KBC/keyboard/AUX, VADP/display, ROM/profile topology, media, and
reset/finalize lifecycle.  It must distinguish:

- accepted selected-device behavior from unselected device breadth;
- deterministic due-event/transaction ordering from instruction timing,
  wait-state, physical bus arbitration, prefetch, and pin-waveform fidelity;
- current-gate evidence from diagnostic, media, host-observation, and future
  product evidence; and
- a documented deferred boundary from an unowned or contradictory claim.

The audit also checks public/product/runtime boundaries, outstanding code
quality and fixture debt, the M5/M6 handoff, and all Queue/TODO/history links.
It must re-read source and evidence rather than infer closure from task titles.

## Bounded sequence

| Subtask | Bounded result |
| --- | --- |
| S1 | Build one source-to-owner-to-evidence ledger for devices, bus/port transactions, event scheduling, reset, current-gate, and physical-L3 residuals. Classify every item as accepted, contradictory, unproved, or deferred. |
| S2 | Reconcile every non-accepted row: fix only documentation contradictions in the audit surface, and create ordered Queue proposals or precise TODO receivers for implementation work. No device implementation is folded into this audit. |
| S3 | Re-run the closure sweep, verify topology and evidence links, close the audit only if no unclassified row remains, and state whether M5 can close or which dependency prevents it. |

## Completion standard

M5 closes only if the final ledger proves every required core/product boundary
or names an owner-approved successor outside M5.  If physical timing, device
breadth, product composition, or compatibility remains unproved, the audit
must keep M5 open and leave a finite, ordered implementation program.  It must
not invent device behavior, make a Windows claim, import media, or weaken
current-gate evidence.
