# Project Status

## Current Work

**Active: M5 T399.**

## M5 T399 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; M5 T399 S3. |
| Admission And Approval | Owner approved the DeskPro L3-first sequence in this conversation on 2026-08-17; execute the already accepted T399 reference-derived bridge before CPU audit. No exception authorizes physical-L3 promotion, reference-source import, ROM/media import, or a Core/VM interface change. |
| Objective | Consume the complete B2 FDC command-to-DRQ/DMA2/IRQ6 batch with an asset-free Model-40 probe, reconcile it to the sole exact PCjs DeskPro configuration, and repair only a demonstrated existing-owner defect. |
| Non-goals | No hardware-time scalar, physical-media/electrical claim, generic-AT substitution, external trace/config/source import, new Core/VM ABI, or whole-T399/L3 decision. |
| Reference Baseline | `docs/etc/evidence/t399-s1-reference-derived-timing-convergence-ledger.md` B2; PCjs `c7f21b4fa2bdedac3d5c73094a6402fdc8b24c70` exact DeskPro 386 EGA/2 MiB configuration; T386 S24 Model-40 FDC logical contract; T397/T398 physical boundary. |
| Candidate Proposal | [DeskPro 386 reference-derived timing bridge](../proposals/m5-deskpro-386-reference-derived-timing-bridge.md). |
| Files And ABI Surface | `tests/machine/vm_model40_fdc_s24_smoke.c`, existing `src/core/machine/fdc.*`, `dma.*`, `pic.*`, and existing VM Model-40 composition only if a focused defect requires it; `docs/etc/evidence/t399-s3-*`, `docs/etc/README.md`, and `docs/states/CURRENT.md`. No public interface change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: one batch, full batch disposition, actual-diff review and acceptance; `docs/rules/ARCHITECTURE.md`: Core owns shared FDC/DMA/PIC semantics and VM owns Model-40 composition; `docs/rules/CODING.md`: C11 focused owner test; `docs/rules/DOCUMENT.md`: indexed evidence/current packet; source policy: reference is read-only and is not imported. |
| Verification | Build `core-machine-fdc-smoke`, `core-machine-fdc-media-change-port-smoke`, `vm-model40-fdc-s24-smoke`; run their exact current CTests; run documentation governance and `git diff --check`. If code changes, run the relevant full current gate before acceptance. |
| Expected Markers | Existing `M5:T386:S24:FDC-12MB-LOGICAL:OK`, `M5:T386:S24:FDC-DMA2-IRQ6:OK`, `M5:T386:S24:MODEL40-FDC-BINDING:OK`; new T399 marker only if a bounded project-owned observation is added. |
| Asset Needs | None. The probe uses synthetic project-owned ROM and floppy bytes. No O:\assets, firmware, guest media, ROM, reference configuration, trace, or binary is read or copied. |
| Reporting Requirements | Record command acceptance, DRQ eligibility, DMA2 transfer, IRQ6 assertion/clear, cold reset and mid-command cancellation; state PCjs-only confidence and 86Box non-corroboration; update B2 ledger before/after and provide an acceptance audit. |
| Stop Conditions | Stop and retain B2 if the exact reference route is generic-only, a checkpoint cannot be observed with the project-owned probe, a behavior conflicts with the frozen topology, or a proposed conversion would turn reference ticks into physical time. |
| Exit Criteria | B2 is accepted only with a complete normalized, asset-free result and focused regression proof, or transferred as unavailable/contradictory with its existing physical-observable receiver. |
| Original Owner Request | “好的，那么接下来开始做T399，力争在CPU指令全面审计之前，把DeskPro 386能做的L3时序部分都做了；如果有权威资料，直接实现；如果没有权威资料但是只有pcjs / 86box / mame / qemu / bochs等，可以参考实现；如果参考都没得参考，加入TODO；如果有需要等待CPU 全面审计之后才能做的，那也等到CPU审计之后的下一任务。” |
| Similar-Issue Sweep | Inspect all Model-40 FDC producer/consumer and reset/cancellation tests plus Core FDC/DMA/PIC routes; classify only selected Model-40 B2 paths and transfer any physical timing gap to the existing physical-observable TODO. |
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
| T399 S2 | Accepted: B3 corrects DeskPro active-low port-61 IOCHK/fail-safe NMI and high-pulse latch clear; physical propagation remains unresolved. [Acceptance audit](../etc/evidence/t399-s2-b3-acceptance-audit.md). |
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
