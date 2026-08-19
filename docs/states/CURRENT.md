# Project Status

## Current Work

**Idle.**

## Current Technical Baseline

- **Current developer artifact:** T431 S1 `vm-0-5-0431` / `build/output/nxvm_0_5_0431.exe`, SHA-256 `63989D0E151497D5DDCAB99BF7EEEADC3300AEB8FFC19BC47A4042D87D9E0723`. T431 adds one Core-owned frozen FDC unready-READ completion policy: Model-40 selects the reference-derived `ST0=48h` result through the existing IRQ/result path, while generic PC/AT retains its policy; physical timing remains transferred.
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
| T431 | Closed: Model-40 selects a reference-derived unready-drive READ `ST0=48h`/IRQ/result completion through one Core policy; generic PC/AT stays generic, and physical FDC/L3 timing remains transferred. [Evidence](../etc/evidence/t431-s1-deskpro-fdc-not-ready-reference.md). |
| T430 | Closed: original/reference-backed Compaq WD 40 MB C:/D: selection now uses frozen Core media slots with optional Model-40 secondary backing; generic ATA stays unchanged, and physical/L3 timing remains transferred. [Evidence](../etc/evidence/t430-s1-deskpro-dual-fixed-disk.md). |
| T429 | Closed: generic-AT CECG port/aperture wait skeleton and D4-memory classification; physical CECG and monitor behavior remain TODO. [S1](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md), [S2](../etc/evidence/t429-s2-d4-cecg-memory-class.md), [S3](../etc/evidence/t429-s3-cecg-aperture-wait.md).
| T428 | Closed: Model-40 selects a generic-AT Core prefetch reservation lifecycle with reset/HOLD/refresh priority; it publishes no physical cycle, page hit or timing result. [Evidence](../etc/evidence/t428-s1-generic-prefetch-reservation.md). |
| T427 | Closed: original-source CPU BUSRDY gate holds the existing Core external completion wait without retirement; Model-40 selects it and 5170 is isolated. [Evidence](../etc/evidence/t427-s1-deskpro-cpu-busready-gate.md). |
| T426 | Closed: independent audit retains all tier-labelled logical receivers but rejects DeskPro physical/L3 acceptance pending named source/corpus blockers. [Evidence](../etc/evidence/t426-s1-deskpro-physical-l3-reaudit.md). |
| T425 | Closed: reconciled FDC/HDC/KBC/DMA/PIC/NMI logical device phases at declared tiers; physical rates and propagation remain transferred. [Evidence](../etc/evidence/t425-s1-deskpro-device-phase-reconciliation.md). |
| T424 | Closed: VM Model-40 BYOB now proves synthetic manifest validation, immutable mapped reset-vector execution and cold-reset replay; vendor POST/service/timing corpus remains external. [Evidence](../etc/evidence/t424-s1-model40-byob-reset-lifecycle.md). |

## Recent Governance

- **M5 Td S107 P1:** retired the completed tiered-fallback foundation from the active Queue, promoted the DeskPro physical-cycle program, and ordered CPU-to-board timing, device phases, Compaq fixed disk, CECG, BYOB lifecycle, and independent L3 re-audit with retained evidence tiers.

- **M5 Td S106 P1:** corrected the retained PCjs DeskPro HDC configuration fact to two generic AT Type-1 10.16 MB drives; the Compaq WD 40 MB non-qualification and all physical receivers remain unchanged.

- **M5 Td S105 P2:** restored a usable tiered DeskPro work path, recorded the five-receiver ledger and admitted next target T406 for a labelled generic-AT KBC serial-delivery mechanism; this is not a physical/L3 acceptance.

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
