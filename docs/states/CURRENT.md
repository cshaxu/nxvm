# Project Status

## Current Work

## M5 T375 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T375 S3 is accepted in the compact progress row below. |
| Admission And Approval | Owner: user, continuing the approved pre-Windows L3 program. T375 S3 identified and allocated this shared core repair; the existing T375 proposal permits shared semantic repairs at their earliest owner. |
| Objective | Repair the shared core time-publication mechanism: a still-halted refresh must not retire retained instruction cost, and one explicit production operation must publish bounded virtual source ticks through the existing elapsed-time/scheduler path. |
| Non-goals | No Model-339 composition source policy, host-clock conversion, run-loop pacing decision, CPU instruction-timing change, board wait/device scalar, device functional repair, raw-IMG sidecar work, ROM/media import, external-reference scalar, or final L3 claim. |
| Reference Baseline | Accepted T375 S3 `9f6aab6d` / `4c1d98b7`, and accepted S2 Model-339 RTC/PIT profile contract. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | `src/core/machine/machine.c`, `machine_interface.h`, possibly trace contract and one focused core-machine smoke/CMake target; indexed evidence and packet. One bounded core production operation is allowed; no test-only public API. |
| Applicable Rules | `docs/rules/ARCHITECTURE.md`: core owns the sole mutable machine-time publisher and composition remains the future source-policy owner; evidence: one internal publication helper and no adapter guest mutation. `docs/rules/CODING.md`: explicit bounded operation and no duplicate test seam; evidence: public contract plus focused replay. `docs/rules/DOCUMENT.md`: indexed evidence only. `docs/rules/EXECUTION.md`: full P1 proof, review and acceptance. |
| Verification | Add a focused replay proving HLT's second no-interrupt run publishes no instruction/time, explicit virtual source ticks advance the same scheduler/RTC/PIT path once, reset restores state, invalid/lifecycle inputs leave state unchanged, and existing rational-clock/timeline/HOLD focused tests still pass. Run documentation governance and `git diff --check`. |
| Expected Markers | `M5:T375:S4:EXPLICIT-MACHINE-TIME:OK`, plus existing rational-clock, timeline and arbitration/HOLD markers. |
| Asset Needs | No asset, host timer, ROM, media, binary, or third-party code required. |
| Reporting Requirements | Report the precise public operation semantics, HLT correction, proof, and the explicit transfer of composition source/pacing policy. |
| Stop Conditions | Stop and transfer if a source policy requires host-time conversion, if the existing scheduler cannot be reused without a second timing owner, if interrupt delivery requires a separate CPU timing decision, or if a compatible public operation cannot have bounded lifecycle semantics. |
| Exit Criteria | A core production virtual-time operation is lifecycle/overflow validated and reuses the sole publication helper; a halted non-interrupt refresh publishes neither a retired instruction nor time; focused replay proves explicit scheduling/reset behavior and no test-only entry is added; Model-339 source policy remains visibly open. |
| Original Owner Request | Owner-directed continuation: implement the current plan to full L3 precision and stop before Windows 3.1; use 86Box/MAME/PCjs only as secondary cross-checks when primary manuals do not determine an instruction timing. |
| Similar-Issue Sweep | Inspect all `elapsed_ticks` writes, all scheduler calls, `core_machine_run` exit paths, lifecycle validation, trace publication, execution-provider time callback, CPU HLT/interrupt refresh, test fixtures and all core-machine public mutation operations. |

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
| T375 S3 | Accepted P1 `9f6aab6d`: the audit proves HLT currently republishes retained instruction cost from the host loop; explicit production virtual-time publication is the next shared repair. [Inventory](../etc/evidence/t375-s3-model339-nonretirement-time-inventory.md). |
| T374 | Closed at `f742433c`: S1--S19 complete the selected Model-339 functional closure and preserve raw-IMG 765 Deleted/Control-Mark/Scan as explicit TODO debt. Board/device timing, final Model-L3, DeskPro 386, PC/XT and Windows 3.1 remain open. [Closure audit](../etc/evidence/t374-s19-task-closure-audit.md). |
| T373 | Closed at `06246a8e`: S1--S4 freeze the three-machine source-labelled capability ledger and exact functional/timing/current-product receivers. **5170, DeskPro Model 40 and PC/XT 5160-268 L3 are not ready.** The next candidate is 5170 selected-device functional closure. [Closure audit](../etc/evidence/t373-s4-task-closure-audit.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |

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
