# Project Status

## Current Work

M5 T350 S1 - PC/AT platform-signals manual-to-source completeness ledger
(Single-Session Mode).

## M5 T350 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continuous holistic device/L3 completion after T349 closure `f9225093` on 2026-08-13. |
| Objective | Build the finite Intel 8254, Motorola MC146818A, and IBM PC/AT signal ledger: counters/gates/output, channel-0 IRQ0, channel-2/port-61/PPI/speaker, calendar/periodic/update/alarm/register-C/IRQ8, CMOS index-NMI mask, and parity/I/O-channel NMI sources. Map each row to source, reset/finalize, consumer, proof, S2--S4 receiver, or named transfer. |
| Non-goals | No speculative speaker/PPI or NMI implementation, host audio/wall clock, RTC-to-NMI route, CPU delivery redesign, firmware rewrite, arbitrary board wiring, external source import, x87, or Windows claim. |
| Reference Baseline | `f9225093` / T349 closed; [platform-signals proposal](../proposals/m5-pcat-platform-signals.md), [T346 L3 audit](../etc/evidence/t346-s1-core-machine-device-l3-audit.md), [T349 lifecycle evidence](../etc/evidence/t349-s4-pcat-pic-lifecycle.md), and [PC/AT profile boundary](../etc/architecture/pc-at-profile.md). |
| Candidate Proposal | [PC/AT platform signals: PIT, PPI, RTC, and NMI](../proposals/m5-pcat-platform-signals.md). |
| Files And ABI Surface | Expected: one S1 ledger, supporting-document index, Current, T350 history, proposal/Queue wording only if the ledger exposes a receiver correction. No runtime, public ABI, or external asset change. |
| Applicable Rules | Task Reading Set; deterministic `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` order; one owner per mutable signal; PC/AT profile policy stays in VM; source policy; primary-manual and project-owned probe discipline. |
| Verification | Reproduce the source/port/timeline/reset/finalize/consumer sweep mechanically; cross-check current PIT/RTC/CMOS/NMI tests and primary manuals; verify every adopted or transferred row once, proposal/Queue links, governance, and diff check. |
| Expected Markers | Retain PIT waveform/readback/divider, RTC/CMOS, NMI delivery, T346 timeline, and T349 PIC lifecycle markers; S1 adds a ledger rather than a runtime marker. |
| Asset Needs | Published Intel 8254, Motorola MC146818A, and IBM PC/AT technical-reference material only; no firmware, guest media, or copied reference source. |
| Reporting Requirements | Record manual authority, implementation owner, writer/reader, signal direction, timing/reset/finalize semantics, proof, residual risk, and exact next receiver for every row. |
| Stop Conditions | Stop for a needed generic CPU/NMI delivery change, host-time/audio integration, source-provider ABI change, ambiguous PC/AT board wiring, third-party source import, or an unbounded new device. |
| Exit Criteria | Every selected platform signal is classified as accepted proof, S2--S4 receiver, or explicit Queue/TODO transfer with admission condition; no current source/consumer/lifecycle route is unowned or unclassified. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at deterministic L3 before deciding any Windows execution route. |
| Similar-Issue Sweep | Inspect all PIT/RTC/CMOS/NMI/PPI registrations, port callbacks, gates/output transitions, IRQ/NMI assertions, timeline advance entries, reset/finalize routes, VM profile declarations, and existing device tests. |

## Current Technical Baseline

- **Current developer artifact:** T346 selects `vm-0-5-0346` /
  `build/output/nxvm_0_5_0346.exe`; its closure-build SHA-256 is
  `7715C8C290969A99CCC1137D2DEEC64421FF245A22BD35287A2AED0C75A8E260`.
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
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |
| T342 | Closed the 80386DX profile ledger: T340 form and T341 state evidence reconcile every assigned row once, and the closure map now names the accepted state owners instead of stale future transfers. Only explicit post-80386, x87, timing/device, and Windows boundaries remain external. [Closure audit](../etc/evidence/t342-s1-80386dx-profile-closure-audit.md). |

## Recent Governance

- **M5 Td S83:** corrected the NXVM self-sibling instruction in `AGENTS.md`;
  renamed the VM-platform injected test macro to
  `VM_PLATFORM_TEST_FAILURE_STAGE` across CMake, implementation, and smoke
  sources; and aligned CMake's project identity and diagnostics with NXVM.
  Documentation self-test, default governance check, exact macro/search audit,
  and diff check passed. Td work has no intended runtime or artifact behavior
  change.

- **M5 Td S82:** restored all 19 historical NXVM README screenshots as static
  documentation assets; rebuilt the public README around the current CMake
  path and truthful NXVM-first, future-NXVDM boundary; and aligned the current
  design/rule wording and README schema self-test. Documentation self-test,
  default governance check, README-link check, and diff check passed. Td work
  has no runtime or artifact change.

- **M5 Td S81:** withdrew the duplicate 80286 `LOCK` candidate after the
  T339 closure audit reconfirmed that accepted T328 already owns the complete
  pre-386 prefix-policy matrix. The Queue now advances directly from T339 to
  the 80386DX form closure; current proposals and T339 transfer ledgers name
  T328, T341, and T342 truthfully. Documentation governance and diff check
  passed. Td work has no runtime or artifact change.

- **M5 Td S80:** reordered the four-profile CPU-completeness Queue into
  dependency-bounded 8086/80186, 80286 descriptor-transfer, 80286 `LOCK`,
  80386DX form, 80386DX state, 80386DX audit, and cross-profile closure
  candidates; added the corresponding unnumbered proposals. Documentation
  governance and diff check passed. Td work has no runtime or artifact change.

- **M5 Td S79:** reordered the M5 CPU-completeness program around a
  four-profile audit, shared delivery foundations, 8086/80186, 80286, and
  80386DX closure candidates, then cross-profile verification. Each candidate
  has a linked unnumbered proposal; no implementation task was allocated.
  Documentation governance, Queue-link verification, and diff check passed.

- **M5 Td S77:** moved the closed T323/T325/T328 proposals into matching
  history companions; made every current proposal require a Queue link and
  added the orphan-proposal negative self-test; compacted repeated
  mechanism-defect requirements into role-specific authorities; clarified
  historical terminology retention and the idle T332 artifact wording.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S76:** retired the closed T330 task-switch debt from TODO and the
  unqueued proposal surface; retained its proposal, debt report, history, and
  matrix as explicitly historical/non-current evidence; and corrected the
  stale T330 closure summary. Documentation governance and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S75:** reconciled Queue state with recorded closures: removed only
  the T323 protection/privilege, T325 paging, T328 legacy LOCK, and T330
  transition-unification candidates; retained the remaining candidate order.
  Documentation governance and diff check passed. Td work has no runtime or
  artifact change.
