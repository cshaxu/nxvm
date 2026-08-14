# Project Status

## Current Work

M5 T352 S5 - selected digital-video composition, artifact, and closure
(Single-Session Mode).

## M5 T352 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 implementation on 2026-08-13; T352 S1--S4 are accepted at `6e68050b`, `0914e475`, `30dec326`, and `8a374fcd`; S1 assigns selected cross-mode/reset/timeline/copy composition and artifact closure to S5. |
| Objective | Compose the selected text/CGA/EGA VADP state graph and prove mode transition, reset/re-arm, deterministic VADP timeline placement, and copied core-to-presentation publication have no stale frame or host mutation path; build and hash-verify the current developer artifact, reconcile every remaining selected ledger row, and close T352 or transfer it precisely. |
| Non-goals | No VGA/VBE/SVGA, unselected EGA or CGA CRTC/mode breadth, composite, host renderer cadence/event-loop policy, firmware expansion, generic mailbox API change, public display/profile ABI change, Windows execution, or x87. |
| Reference Baseline | `8a374fcd`; [T352 proposal](../proposals/m5-vadp-digital-video-completeness.md), [S1 ledger](../etc/evidence/t352-s1-digital-video-ledger.md), S2--S4 evidence, T346 S5 timeline boundary, current session display/mailbox and VADP tests. |
| Candidate Proposal | [PC/AT digital video completeness](../proposals/m5-vadp-digital-video-completeness.md). |
| Files And ABI Surface | Expected: focused composition smoke, private VADP/session display implementation only if a reproduced defect requires it, evidence/history/Current, and CMake artifact identity. No platform renderer, mailbox/public display ABI, profile/ROM API, generic timeline, or host-event-loop redesign. |
| Applicable Rules | VADP remains the sole guest display-state owner; CGA/EGA layout branches remain separate only for their real format differences; deterministic timeline advances VADP after KBC; core snapshot and presentation mailbox are value-copy boundaries; reset cancels/reschedules guest callbacks and clears transient display state without host mutation. |
| Verification | Sweep VADP mode writers/capture/reset, `core_machine_peripheral_tick`, session capture/publish/mode notification, mailbox publish/capture/finalize, and platform consumers. Prove text-to-CGA-to-EGA-to-reset/re-arm composition, captured and mailbox frame isolation, no stale dimensions/pixels/palette, VADP-after-KBC deterministic trace order and reset rescheduling, artifact SHA, retained core/VM/ROM/DOS display proofs, governance, diff check, and full current gate. |
| Expected Markers | Retain T346 input/display timeline plus selected CGA/EGA/core/VM/ROM/DOS markers; add one S5 composition marker only if no retained owner proves cross-mode and mailbox isolation together. |
| Asset Needs | Existing project source/evidence, current developer build, and already admitted firmware/media tests only; no new guest media, source import, host capture, or renderer asset. |
| Reporting Requirements | Record cross-mode writer/reader/copy/reset/timeline matrix, actual defect and caller sweep if any, artifact identity/SHA, exact transfer of unselected modes and host timing, focused/system/DOS evidence, and T352 closure audit. |
| Stop Conditions | Stop for a public mailbox/renderer/profile ABI change, generic timeline defect outside VADP placement, required unselected EGA/VGA/CRTC feature, host scheduling policy, firmware/Windows dependency, or ambiguity about selected display ownership; transfer rather than broaden. |
| Exit Criteria | Every selected S1 digital-video row is accepted or has one exact later receiver; mode/reset/copy/timeline composition has direct proof with no stale or aliased frame state; the current developer artifact is built and hash-verified; T352 history/proposal/Queue/Current agree on closure and transfers. |
| Original Owner Request | Make CPU/fpu-external core-machine devices, chips, buses, and ports comprehensive and stable at deterministic L3 before choosing a Windows execution route. |
| Similar-Issue Sweep | Sweep VADP capture/reset/finalize and all selected mode writers; machine peripheral/reset/timeline paths; session display capture/publish/mode callback; mailbox/platform consumers; display core/VM/ROM/DOS tests; T346 evidence; T352 ledger/history/proposal/Queue/TODO and artifact inputs. |

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
| T352 S4 | Accepted selected EGA planar path: one VADP owner for registration, indexed controllers, aperture/latches/planes, 320 and mode-10 capture, reset/re-arm, and copied output. [Evidence](../etc/evidence/t352-s4-ega-planar-path.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |

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
