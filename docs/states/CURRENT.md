# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T375 S15. |
| Admission And Approval | The owner approved continued T375 execution, external owner-managed ROM research outside Git, and standing implementation/push authority. This S may derive a declarative hardware-state result from the owner-authorized IBM 5170 Rev.3 ROM observation but must not commit ROM bytes, paths, hashes, a ROM catalogue, or make the ROM a runtime dependency. |
| Objective | Bind the selected IBM 5170 Model 339 firmware route to its observed Rev.3 80-column CGA CRTC default programming through the existing firmware port boundary. Keep generic default-PC/AT EGA behavior unchanged; prove exact selected CRTC state, BDA-visible 80-column text state, and retained adjacent EGA behavior. |
| Non-goals | No ROM mapping/BYOB provider, ROM or guest-media import, generic profile alteration, factory claim beyond the selected Rev.3 abstract firmware slot, full INT 10h mode-table implementation, R8 interlace/skew support, physical CGA waveform/timing, ISA contention/snow, monitor output, test-only API, or 5170/M5 L3 claim. |
| Reference Baseline | The Model-339 descriptor selects the abstract IBM 5170 Rev.3 slot; T375 S11 owns non-interlaced CRTC state, and S12--S14 establish selected CGA identity, constrained character cadence and phase crosswalk. Owner-authorized local Rev.3 firmware observation establishes the declarative default sequence; no firmware image enters NXVM. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Expected: `docs/states/CURRENT.md`, indexed evidence and `docs/etc/README.md`, `src/vm/profile/default_profile/firmware/context.h`, `src/vm/composition/session/session.c`, `src/vm/profile/default_profile/firmware/qdcga.c`, and the existing Model-339 CGA topology smoke. No public ABI or test-only capability. |
| Applicable Rules | `docs/rules/EXECUTION.md`: active packet, pushed P1/review/P2 and actual-change inspection. `docs/rules/ARCHITECTURE.md`: VM composition supplies the selected firmware capability; QDCGA remains the existing firmware owner and VADP owns mutable registers. `docs/rules/CODING.md`: narrow owner-local context addition, no duplicate firmware route. `docs/rules/DOCUMENT.md`: Current is active authority and evidence is indexed. `docs/etc/operations/policy/source-policy.md`: firmware remains external and conclusions are neutral behavior only. |
| Verification | Create Model-339 and generic sessions. Assert only Model-339 receives the selected R0--R7/R9/R10--R15 values through VADP's existing CRTC port route, retains BDA mode 3/80 columns/page/cursor behavior, and starts its logical raster; assert generic EGA topology/controller state stays unchanged. Replay Model-339 clock, VADP status, and adjacent EGA smokes; run documentation governance. |
| Expected Markers | `M5:T375:S15:MODEL339-REV3-CGA-DEFAULTS:OK` plus retained topology, clock and VADP markers. |
| Asset Needs | Owner-managed IBM 5170 Rev.3 ROM is read-only research only and remains outside Git. Do not commit its bytes, path, hash or download mechanism. |
| Reporting Requirements | Report the selected-only firmware binding before implementation, push P1 with focused proof/evidence, independently review actual changes, then push P2 acceptance. State all retained physical/default-mode-table transfers. |
| Stop Conditions | Stop and transfer if a default sequence cannot be distinguished from a generic virtual-BIOS behavior, depends on copied firmware/code, needs a new public/test interface, changes generic EGA, or requires a physical timing/monitor claim. |
| Exit Criteria | Model-339-only declarative defaults pass focused selected/generic regressions; evidence records source containment and remaining boundaries; no external asset/dependency enters NXVM and documentation governance passes. |
| Original Owner Request | Continue to comprehensive L3 before Windows 3.1; use authorized ROMs and secondary emulators only as bounded research inputs; preserve the no-test-API and external-asset boundaries. |
| Similar-Issue Sweep | Sweep every QDCGA reset caller, firmware-context constructor, profile selector, CRTC port route, BDA writer, selected-model topology test and default EGA regression so no generic or second firmware source is created. |

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
| T375 S8 | Accepted P1 `7f9200e7`: IBM CGA 3DA bit-0 buffer-access semantics are corrected without changing EGA behavior; full 6845 raster and source-derived CGA phase remain open. [Reconciliation](../etc/evidence/t375-s8-model339-cga-clock-reconciliation.md). |
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
