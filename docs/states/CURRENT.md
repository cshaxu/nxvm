# Project Status

## Current Work

## M5 T375 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user. Approval: persistent instruction to continue the approved L3 program, including use of 86Box/MAME/PCjs only as secondary sources when primary manuals leave a range or none. Scope is the selected Model-339 digital CGA clock/raster contract only. |
| Objective | Reconcile primary IBM CGA/PC/AT documentation with the current VADP text/raster callback, determine whether a source-backed Model-339-compatible CGA time conversion and status/frame contract can be admitted, and implement it only if the complete selected behavior can be bounded. |
| Non-goals | Do not infer physical CGA contention, composite/analog output, an unselected adapter, host display refresh, a generic AT video scalar, or a final L3 result. Do not turn reference implementation behavior into a board fact. |
| Reference Baseline | T375 S1/S7 generic VADP-cadence finding, T374 selected CGA functional closure, the frozen Model-339 CGA-only topology, and the Model-339 board/phase proposal. |
| Candidate Proposal | [M5 5170 board/phase timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Primary-source/evidence record first; inspect Model-339 descriptor, VADP clock/raster/status implementation and focused CGA tests. Code and profile changes are admitted only by the source result and must use existing clock/time ownership. |
| Applicable Rules | Source policy requires no unreviewed third-party import; IBM/manual facts lead. Architecture preserves core scheduler/VADP ownership and VM profile selection; coding forbids a device-local timer/test path; documentation indexes source and transfer evidence. No exception requested. |
| Verification | Build a cited source-to-current-route matrix for oscillator, CRTC mode geometry, status/retrace and frame cadence; sweep all VADP advance/status/config consumers; if implementation is admitted, prove rational conversion/reset/status effects with focused tests. Otherwise prove a precise reference-exhausted transfer. Run documentation governance. |
| Expected Markers | `M5:T375:S8:MODEL339-CGA-CLOCK-RECONCILIATION:OK` and documentation-governance success. |
| Asset Needs | Consult public primary manuals only; no firmware, media, external source code or reference executable is imported. |
| Reporting Requirements | Record edition/page/source status, model-versus-adapter boundary, current implementation mapping, exact admitted or rejected scalar, test evidence and remaining physical transfers. |
| Stop Conditions | Stop before implementation if selected Model-339 adapter/monitor or source-to-core conversion cannot be bounded, or if a result requires a physical contention/waveform measurement or a scope/proposal change. |
| Exit Criteria | CGA oscillator/raster/status facts and current routes are fully reconciled; any implementation has a source-backed owner/reset/proof; otherwise its explicit transfer names the missing fact; no generic or L3 claim is made. |
| Original Owner Request | Continue toward comprehensive L3 before Windows 3.1, using 86Box/MAME/PCjs only as secondary cross-checks where primary authority is incomplete. |
| Similar-Issue Sweep | Cover Model-339 versus default-PC/AT descriptors, VADP text/graphics/status/raster/reset/copy routes, clock-plan consumers, all tests and every direct advance/status call. |

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
| T375 S7 | Accepted P1 `eba64b24`: complete callback/domain/service ledger confirms only PIT and RTC have Model-339 source conversions; generic cadence remains an explicit transfer rather than false timing. [Ledger](../etc/evidence/t375-s7-model339-callback-clock-ledger.md). |
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
