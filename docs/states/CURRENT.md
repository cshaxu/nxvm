# Project Status

## Current Work

M5 T353 S1 - PC/AT selected port-topology, route, and lifecycle ledger
(Single-Session Mode).

## M5 T353 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continuous holistic device/L3 implementation on 2026-08-13 and authorized execution after T352 closure; this packet admits the first Queue candidate as T353 S1. |
| Objective | Produce a complete selected PC/AT manual/profile/composition/core/proof ledger for declared and actually registered port ranges, IRQ/DMA routes, reset/finalize ownership, and firmware-visible topology; allocate every row to T353 S2--S4 or one exact transfer. |
| Non-goals | No unapproved device implementation, generic port framework, host passthrough, serial/parallel/game/MIDI/network/SCSI/PCI/ISA-PnP, VGA/VBE, speaker/PPI implementation without corpus, physical bus timing, Windows execution, or x87. |
| Reference Baseline | `a8636687`; [T353 proposal](../proposals/m5-pcat-port-topology-and-peripherals.md), T346--T352 histories/evidence, default PC/AT profile, machine/session composition, current controller and port tests, and the IBM PC/AT reference admitted as documentation research only. |
| Candidate Proposal | [PC/AT port topology and selected peripheral completion](../proposals/m5-pcat-port-topology-and-peripherals.md). |
| Files And ABI Surface | Expected: evidence/history/Current and a focused static or owner ledger test only if required. No production/public port/profile/platform ABI change in S1. |
| Applicable Rules | The profile declares composition topology; each registered range, command state, IRQ/DMA route, and lifecycle has one owner; core owns guest port dispatch/time; platform code cannot mutate guest device state; absent devices need exact corpus-gated transfer, not a false support claim. |
| Verification | Sweep profile port/route declarations, session and machine-device wiring, core port registrations, controller reset/finalize/timeline paths, firmware writers, current port/topology/ownership tests, Queue/TODO. Produce a row-by-row ledger with manual/source/proof/receiver classification; run affected focused gates, governance, diff check, and full current gate. |
| Expected Markers | Retain selected PIC/PIT/DMA/KBC/VADP/CMOS/FDC/HDC/topology markers; add an S1 marker only if a mechanical owner-ledger verifier is necessary. |
| Asset Needs | Existing source, project evidence, and documentation research only; no firmware/media download, third-party source, host device, or Windows corpus. |
| Reporting Requirements | Record every declared and registered range, owner, IRQ/DMA, reset/finalize and proof; name any actual mismatch and caller sweep; distinguish selected completion from deferred peripherals; propose bounded S2--S4 receivers. |
| Stop Conditions | Stop for a public profile/port ABI change, a generic registration/IRQ/DMA framework defect, required unselected peripheral implementation, ambiguous PC/AT hardware contract, host passthrough, firmware/Windows dependency, or source import. Transfer or seek approval rather than broaden. |
| Exit Criteria | Each selected declared or registered port row has one truthful owner/proof/receiver disposition; no ambiguous profile-to-registration or lifecycle route remains; every missing optional device has a bounded TODO/Queue admission condition; S2--S4 scope follows the evidence rather than an address-list patch. |
| Original Owner Request | Make CPU/fpu-external core-machine devices, chips, buses, and ports comprehensive and stable at deterministic L3 before choosing a Windows execution route. |
| Similar-Issue Sweep | Sweep PC/AT descriptor ranges/routes, session storage/device composition, machine configure/assembly/reset/finalize, port registry, PIC/DMA/timeline consumers, selected controller/profile/firmware tests, T346--T352 evidence, Queue, TODO, and any source/manual references. |

## Current Technical Baseline

- **Current developer artifact:** T352 selects `vm-0-5-0352` /
  `build/output/nxvm_0_5_0352.exe`; its closure-build SHA-256 is
  `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`.
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
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; `vm-0-5-0352` SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |

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
