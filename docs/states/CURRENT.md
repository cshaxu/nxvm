# Project Status

## Current Work

**Idle.**

## Current Technical Baseline

- **Current developer artifact:** T421 S1 `vm-0-5-0419` / `build/output/nxvm_0_5_0419.exe`, SHA-256 `859E1B93C6891E8EAAF0D98D4DBEF25F2383F911EC243390A50FB9A9CDBBA5BF`. T421 adds one Core-owned logical port-61h/PIT-channel-2 speaker line: Model-40 D4 and IBM 5170 planar configuration select it without a second provider; host audio and physical acoustic behavior remain transferred.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T424 | Closed: VM Model-40 BYOB now proves synthetic manifest validation, immutable mapped reset-vector execution and cold-reset replay; vendor POST/service/timing corpus remains external. [Evidence](../etc/evidence/t424-s1-model40-byob-reset-lifecycle.md). |
| T423 | Closed: chargeable D4 external CPU-memory waits now advance through a Core-owned non-retiring completion state before retirement; BWAIT/ISA/async-prefetch physical work remains transferred. [Evidence](../etc/evidence/t423-s1-cpu-board-retirement-wait.md). |
| T422 | Closed: original D3PE SLOWD*/refresh/auxiliary-PIT channel-2 bridge uses one Core hold route; Model-40 binds it and 5170 remains isolated. [Evidence](../etc/evidence/t422-s1-d4-slowdown-hold.md). |
| T421 | Closed: Core owns the logical port-61h/PIT-channel-2 speaker line; S2 proves existing planar/D4 port-B exclusivity in both orders. Model-40 and 5170 keep one selected route. [S1](../etc/evidence/t421-s1-d4-speaker-line.md), [S2](../etc/evidence/t421-s2-port-b-exclusivity.md). |
| T420 | Withdrawn as duplicate: T401 S1-S78 already supplies the finite four-profile instruction-correctness audit and T360 S1-S5 supplies the source-timing consumer reconciliation. No second CPU audit path is created. [Closure audit](../etc/evidence/t420-s2-duplicate-audit-withdrawal.md). |
| T419 | Closed: bounded D4 discrete timing mechanisms retain one Core owner and shared Model-40/IBM-5170 selection; the asynchronous physical producer and phase work transfer without a Model-L3 claim. [Closure audit](../etc/evidence/t419-s7-task-closure-audit.md). |
| T418 | Closed: generic-AT locality ends at every instruction boundary; physical D4 overlap timing remains transferred. [Closure audit](../etc/evidence/t418-s2-instruction-boundary-locality-closure-audit.md). |
| T417 | Closed: D4 counter-1 refresh low pulses invalidate generic-AT CPU locality; physical refresh timing and arbitration remain transferred. [Closure audit](../etc/evidence/t417-s2-refresh-locality-closure-audit.md). |

## Recent Governance

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
