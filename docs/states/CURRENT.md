# Project Status

## Current Work

**Active: M5 T399.**

## M5 T399 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T399 S4. |
| Admission And Approval | Owner approved completing every feasible DeskPro L3 reference-derived batch before CPU audit on 2026-08-17; T399 S3 is accepted. No exception permits generic AT substitution, reference import, protected-media use, physical-L3 promotion or Core/VM ABI change. |
| Objective | Consume the complete B4 Compaq HDC command/DRQ/IRQ14 batch by qualifying or rejecting the available exact DeskPro reference route and recording the existing project-owned HDC boundary. |
| Non-goals | No physical media/ECC claim, time scalar, generic AT HDC as Compaq proof, external media/ROM/config/source import, production behavior change without a demonstrated defect, or whole-T399 decision. |
| Reference Baseline | T399 ledger B4; PCjs `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70` DeskPro configurations; T386 S5/S26 Compaq WD 40 MB HDC tests; T397/T398 physical-media transfer. |
| Candidate Proposal | [DeskPro 386 reference-derived timing bridge](../proposals/m5-deskpro-386-reference-derived-timing-bridge.md). |
| Files And ABI Surface | Existing `tests/machine/core_machine_compaq_hdc_s5_smoke.c`, `tests/products/vm_model40_hdc_s26_smoke.c`, `src/core/machine/hdc.*`, existing Model-40 composition, plus indexed T399 evidence and status only. No public ABI change. |
| Applicable Rules | Execution: one ledger batch and full disposition; Architecture: Core owns shared HDC, VM selects Model-40; Coding: focused C11 owner tests; Document: indexed evidence/current packet; source policy: read-only reference only. |
| Verification | Build and run `core-machine-compaq-hdc-s5-smoke`, `core-machine-compaq-hdc-machine-s5-smoke`, and `vm-model40-hdc-s26-smoke`; documentation governance and diff check. |
| Expected Markers | `M5:T386:S5:COMPAQ-HDC-ROUTE:OK`, `M5:T386:S5:COMPAQ-HDC-MACHINE:OK`, `M5:T386:S26:MODEL40-HDC-STARTUP:OK`, and `M5:T386:S26:MODEL40-HDC-FIXED-MEDIA:OK`. |
| Asset Needs | None; tests synthesize inputs. Reference configurations are read-only and are not copied. |
| Reporting Requirements | Record whether PCjs proves the exact Compaq HDC route, command/DRQ/IRQ14/reset/cancel coverage, any disagreement, and the precise TODO receiver if rejected. |
| Stop Conditions | Reject B4 for this tier if the exact configuration binds only an IBM/generic AT HDC, a required checkpoint lacks a project-owned probe, or the result would infer physical media/timing. |
| Exit Criteria | B4 is either accepted with exact reference-derived proof and focused replay, or rejected with the existing Compaq physical-media/T397-T398 receiver; no unclassified route remains. |
| Original Owner Request | Complete all feasible DeskPro L3 timing before CPU instruction audit; direct authority first, qualified emulator references second, otherwise TODO; defer CPU-audit prerequisites. |
| Similar-Issue Sweep | Inspect every selected Model-40 Compaq-HDC and Core HDC test plus PCjs DeskPro configuration references; distinguish controller route from generic Type-5 media and physical-media claims. |
## Current Technical Baseline

- **Current developer artifact:** T395 S1 P4 `vm-0-5-0395` /
  `build/output/nxvm_0_5_0395.exe`, SHA-256
  `5DBD4B50762BAD5E393CD8EFA8A0CB7E06FD9A1304A44F8993CC64915A99D4A6`.
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
| T399 S3 | Accepted: B2 reconciles the selected 1.2 MB FDC→DMA2→IRQ6 logical order, reset and cancellation at PCjs-only reference-derived confidence; physical device timing remains unresolved. [Acceptance audit](../etc/evidence/t399-s3-b2-acceptance-audit.md). |
| T398 | Closed: final exact 1986 DeskPro Model-40 audit is not-ready for L3; five physical/firmware boundaries retain explicit TODO receivers and require a fresh audit after repair. [Closure audit](../etc/evidence/t398-s2-task-closure-audit.md). |
| T397 | Closed: selected functional FDC/HDC/CECG/KBC-NMI/DMA routes and reset/order evidence retained; physical availability, phase and service facts transfer to explicit TODO receivers. Final DeskPro L3 may make only a ready/not-ready decision. [Closure audit](../etc/evidence/t397-s2-task-closure-audit.md). |
| T396 | Closed: finite Model-40 source/owner, six-domain, reset and callback-order ledger retained; all unsourced physical clock, phase, wait and device/firmware visibility facts transfer to the next DeskPro candidate. No physical scalar or L3 claim. [Closure audit](../etc/evidence/t396-s2-task-closure-audit.md). |
| T395 | Closed: private 12-to-64 bounded Core route growth with direct allocation/limit rollback, frozen-registration and order/decline proof. Transfers only the queued DeskPro board-level timing receiver. [Closure audit](../etc/evidence/t395-s2-task-closure-audit.md). |
| T394 | Closed: S1-S7 establish the complete C0 81-key qualification and Core copied-descriptor safety boundary, then transfer physical selection solely to the queued source-backed DeskPro board clock-plan contract. No physical clock or L3 claim. [Closure audit](../etc/evidence/t394-s7-physical-qualification-closure-audit.md). |
| T392 | Closed: S1-S5 exhaust copied C1, VM/session and FDC operation candidates, then repair the missed proposal archival and stale Queue entry; no endpoint or defect exists. Only a separately admitted generic immutable device-operation snapshot plus selected consumer contract can proceed. No Core/VM ABI, CPU timing, physical, board, firmware or L3 claim. [Corrective audit](../etc/evidence/t392-s5-closure-topology-corrective-audit.md). |
| T391 | Closed: S1-S8 establish the bounded C0A startup predecessor, complete C1 aggregate disposition and the truthful transfer that C1 physical qualification remains blocked pending a new finite checkpoint or earliest-owner defect. No Core/VM interface, timing, physical, board, firmware or L3 claim. [Closure audit](../etc/evidence/t391-s8-startup-semantic-closure-audit.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |

## Recent Governance

- **M5 Td S104 P1:** added the owner-authorized reference-derived DeskPro timing-bridge candidate ahead of CPU audit; it preserves physical-L3 not-ready and requires a fresh independent re-audit.

- **M5 Td S103 P1:** adopted task-scale convergence and coverage governance: coverage-bearing T work requires a durable universe, batch dispositions, completion predicate and coordinator reconciliation before whole-domain or L3 acceptance.
- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
