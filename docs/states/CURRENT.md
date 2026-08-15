# Project Status

## Current Work

## M5 T374 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T374 remains the latest open numeric task and S5 follows accepted S4. |
| Admission And Approval | The owner approved completing selected-machine functional gaps before timing/L3 closure and authorized continuing T374. S4 selected the source-determinate 8272A/uPD765 Ready-transition attention repair. |
| Objective | At the existing FDC owner, detect a selected logical-drive Ready transition independently of media generation; publish the existing IRQ6/`flagINTR` path and source-defined ST0 attention state; have `Sense Interrupt Status` consume it; retain DIR disk-change behavior. |
| Non-goals | No controller clock, polling interval, drive motor/settling/index behaviour, DMA service/grant timing, board wait, physical waveform, media import, new device profile, ATA/HDC, MFM/ST-506, EGA/VGA, AUX, or Model-339 L3 claim. No Read Deleted/Write Deleted/Scan implementation. |
| Reference Baseline | T374 S4 FDC contract audit; Intel 8272A primary controller documentation (Ready polling, attention IRQ, ST0 `C0h` interrupt code and Not Ready indication); existing `core_machine_fdc` Ready predicate, IRQ owner, DOR reset and Sense-Interrupt path. |
| Candidate Proposal | [IBM PC/AT 5170 selected-device functional closure](../proposals/m5-5170-selected-device-functional-closure.md). |
| Files And ABI Surface | `src/core/machine/fdc.{c,h}`, cohesive FDC media-change smoke, T374 evidence/history/index, and `docs/states/CURRENT.md`; no new public interface, profile descriptor, ROM/media or runtime configuration. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` implementation/P lifecycle, evidence and similar-issue rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; `docs/design/ARCHITECTURE.md`; `docs/design/CODING.md`; `docs/rules/ARCHITECTURE.md`; `docs/rules/CODING.md`; source policy. The repair retains the single FDC state/IRQ owner and does not derive third-party source. |
| Verification | Extend the FDC media-change smoke to prove: Ready transition after `Specify` raises IRQ6 exactly once; `Sense Interrupt Status` returns Ready-change ST0/PCN and clears the IRQ; media-generation-only change does not create attention; DOR reset clears pending attention; existing DIR disk-change, DMA cancellation and seek/recalibrate paths remain. Build/run focused FDC and Model-339 topology targets through Git-Bash CMake/Ninja; run documentation governance and `git diff --check`. |
| Expected Markers | One per-drive observed-ready state is updated at initialization/reset/refresh; only an actual logical Ready edge publishes one existing FDC IRQ result. No duplicate PIC/IRQ owner, no mutation of transfer/DMA state, and no timing scalar. |
| Asset Needs | None. Primary documentation is read only; no ROM, firmware, guest media, trace, third-party source or local asset is added. |
| Reporting Requirements | Report owner-boundary confirmation, exact ST0/acknowledgement behavior, media-versus-Ready sweep, fresh build/replay, pushed P1, coordinator acceptance and next T374 functional receiver. |
| Stop Conditions | Stop and transfer if attention ordering requires an unprovided controller clock/poll cadence, board signal mapping, multiple pending-interrupt queue, a profile-specific FDC identity, or a source conflict. Do not invent a timing interval or expand into unsupported commands. |
| Exit Criteria | The source-defined Ready-transition attention state is observable once through IRQ6 and `Sense Interrupt Status`, clears through the existing owner/reset route, remains distinct from generation/DIR disk-change, and focused fresh regressions pass. |
| Original Owner Request | Implement each selected machine's functional gaps before timing/L3 closure, treating exact documentation as authority and reference emulators only as fallback; use CMake through Git Bash for local builds. |
| Similar-Issue Sweep | Inspect all FDC Ready/media queries, initialization/reset/finalize, DOR cancellation, IRQ/DRQ publication and all `Sense Interrupt Status` producers/consumers; prove no other caller publishes Ready attention or conflates it with media generation. |

## Current Technical Baseline

- **Current developer artifact:** T369 S4 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S4 closure audit.
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
| T374 S4 | P1 `70627f3c` audits selected 8272A/uPD765 FDC behavior and selects missing Ready-transition attention IRQ / Sense-Interrupt delivery as an owner-local repair; physical signal, DMA and drive timing remain open. [Audit evidence](../etc/evidence/t374-s4-fdc-functional-contract-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine source-labelled capability ledger and exact functional/timing/current-product receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** The next candidate is 5170 selected-device functional closure. [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |

## Recent Governance

- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes the capability-ledger then functional-before-timing
  closure sequence for all three machines, and adds their bounded proposals.
  It records the changed owner media identity truthfully, retains external ROMs
  outside Git, and removes closed T367/T368 work from future Queue positions.
  Documentation governance and diff check passed; no runtime, artifact, or task
  activation change.

- **M5 Td S93 P1:** splits T366's transferred 80286 retirement ledger from
  5170 bus work; orders 5170 CPU, bus, device, phase and audit closure; and
  pre-splits DeskPro 386 into profile/CPU, board/device and audit candidates.
  A new pre-Windows L3 admission audit gates every Windows candidate. The
  Roadmap, exact T366 receivers and downstream dependencies agree.
  Documentation governance and diff check passed; no runtime, artifact or task
  activation change.

- **M5 Td S92 P1:** reconciled M5's executable Queue with closed T362--T365
  history; makes T366's profile-lock, selected-NMI-source, and bus-allocation
  order explicit; and adds a pre-Windows closure for every current-product
  device capability. TODO retains only genuinely unplanned, unsupported, or
  80186-only debt. Documentation governance and diff check passed; no runtime,
  artifact, or task-activation change.

- **M5 Td S91 P1:** made the architecture design explicitly retain one shared
  core decode/execution and CPU/DMA transaction lifecycle across machine
  profiles. 8088 owns only its documented external-bus and prefetch/timing
  difference; 80286/80386 retain only their documented architectural and
  board-local differences. Documentation governance and diff check passed; no
  runtime, artifact, task-activation, or queue change.

- **M5 Td S90 P1:** ordered M5 physical L3 closure as IBM PC/AT 5170 80286,
  exact Compaq DeskPro 386 80386, distinct 8088 CPU profile, and IBM 5150/XT
  8088 before the DeskPro-based Windows 3.1 corpus. It retains standalone
  8086/80186 CPU profiles, adds bounded 8088/DeskPro proposals, and changes no
  runtime, artifact, or task activation. Documentation governance and diff
  check passed.

- **M5 Td S89 P1:** made the queued complete instruction-timing proposal the
  sole current explanation of the shared four-profile audit method and the
  distinct 8086/80186/80286/80386 accounting boundaries. It changes neither
  rules, README, Queue ordering, TODO, runtime, nor the documentation
  topology. Documentation governance and diff check passed.

- **M5 Td S90:** made completed PC/AT device-service timing an explicit
  prerequisite of selected-profile model-L3 phase refinement; retained the
  established Queue order and withdrew the unstarted T362 preparation.
  Documentation governance and diff check passed; Td work has no runtime,
  artifact, or task-activation change.

- **M5 Td S89:** converted the remaining primary-source and PC/AT NMI debt
  into three bounded proposals; ordered authority review before the complete
  timing corpus, and device audit before NMI ownership and physical timing.
  Documentation governance and diff check passed; Td work has no runtime,
  artifact, or task-activation change.

- **M5 Td S88:** reconciled the active T359 mechanism plan after S3 acceptance:
  S4 owns the stateful string/repeat/ordinary-I/O timing mechanism, and the
  secondary, privileged, and closure units follow as S5--S7. Documentation
  governance, the T359 inventory verifier, and diff check passed; Td work has
  no runtime or artifact change.
