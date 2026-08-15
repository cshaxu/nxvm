# Project Status

## Current Work

## M5 T375 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user. Approval: persistent instruction to continue the approved L3 program; this S is a bounded T375 phase-timing audit required before any additional rate or service implementation. No source/media import is approved or needed. |
| Objective | Produce a source-labelled Model-339 callback/clock/service ledger for every selected timing consumer, distinguish a callback cadence from delivered device ticks and physical phase, and allocate the earliest safe repair or explicit transfer for each remaining generic cadence. |
| Non-goals | Do not add host-time coupling, choose an unsupported scalar, alter device function, claim waveform/cycle exactness or Model-339 L3, or absorb the separately queued raw-IMG sidecar work. |
| Reference Baseline | T375 S1 timing inventory, S2 RTC/PIT clock contract, S3--S6 explicit non-retirement time/source boundary, T370 selected-device service transfers, and the Model-339 board/phase proposal. |
| Candidate Proposal | [M5 5170 board/phase timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Evidence/index/CURRENT only unless the audit establishes an owner-approved, source-backed shared repair; inspect core scheduler/clock, Model-339 descriptor, device advance paths and their focused tests. |
| Applicable Rules | Source policy applies to primary-manual and permitted reference research; architecture requires one scheduler/time publisher and no device-local clock; coding requires no test-only path; documentation rules require indexed evidence and no queue/status rewriting. No exception requested. |
| Verification | Static source sweep must account for every Model-339 clock-plan field and all three timeline callbacks; cite source status for each selected consumer; reconcile callback count, delivered tick count, reset phase, wait/HLT progress, and service/visibility order. Documentation governance and diff check must pass. |
| Expected Markers | `M5:T375:S7:MODEL339-CALLBACK-CLOCK-LEDGER:OK` and documentation-governance success. |
| Asset Needs | Primary documentation may be consulted without importing it. No ROM, guest medium, third-party source or executable reference is required. |
| Reporting Requirements | Index an evidence ledger naming every selected consumer, its current cadence/tick route, source disposition, implementation owner and next receiver; state explicitly whether a safe repair is admitted. |
| Stop Conditions | Stop before implementation if a needed rate requires an unselected device/aftermarket-drive identity, a host pacing policy, physical waveform measurement, or a proposal/rules change. Transfer it precisely. |
| Exit Criteria | The full selected clock/service callback graph is accounted for, current generic cadence is not mislabeled as board timing, all repair candidates have a source/owner decision, focused sweep/gate pass, and no L3 claim is made. |
| Original Owner Request | Continue the current task plan to comprehensive L3 precision before Windows 3.1, using 86Box/MAME/PCjs only as secondary cross-checks where primary instruction/device authority leaves a range or no range. |
| Similar-Issue Sweep | Cover DMA/PIT/PIC, FDC/HDC/RTC, KBC/VADP, provider callback, profile clock copying, reset schedules, normal retirement and S6 external source publication; ensure no other clock or direct device-advance path bypasses the shared scheduler. |

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
| T375 S6 | Accepted P1 `4cc137f8`: VM session now owns an injected virtual-source descriptor that advances machine time only for active, non-step core waits; reset rebases it and host sleep remains non-temporal. [Binding evidence](../etc/evidence/t375-s6-model339-virtual-time-source-binding.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine source-labelled capability ledger and exact functional/timing/current-product receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** The next candidate is 5170 selected-device functional closure. [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |

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
