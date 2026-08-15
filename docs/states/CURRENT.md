# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T375 S9, the next unused subtask of the retained open T375 progress. |
| Admission And Approval | Owner: repository user. Approval: the 2026-08-15 request to change the direct-injection API into explicit test/native-input boundaries while avoiding a test-only API. Scope: correct the production host/native input boundary and its test callers only. |
| Objective | Make the VM host-input ingress and the core attached-device native-input ingress explicit, then migrate integration tests to the production boundary they model. |
| Non-goals | No keyboard/AUX protocol, scan-set mapping, IRQ/FIFO, timing, ROM/media, platform UI, device capability, Model-339 L3, or new test-only public API. Do not add a compatibility alias that preserves an ambiguous direct-injection name. |
| Reference Baseline | Accepted T374 S18 native-keyboard boundary evidence; accepted T375 S1--S8 timing progress; current `core_platform_input_source`, VM request transport, mapper, KBC and platform-adapter routes. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md), limited to the existing 8042/keyboard ingress ownership needed for its later timing work. |
| Files And ABI Surface | Expected: `src/core/machine/machine_interface.h`, `src/core/machine/machine.c`, `src/vm/composition/session/{session_interface.h,session.h,session.c,lifecycle.c}`, affected VM/platform and machine tests, CMake registration only if a new focused smoke is required, `docs/etc/evidence/`, `docs/etc/README.md`, and `CURRENT.md`. Public names and callers may change only to make host versus attached-device semantics explicit. |
| Applicable Rules | `docs/rules/EXECUTION.md`: continuation, one complete implementation P then review P, evidence and similar-issue sweep. `docs/rules/ARCHITECTURE.md`: one input owner/path, opaque bounded public operations and composition-only integration. `docs/rules/CODING.md`: no mirror test state or test-only public contract. `docs/rules/DOCUMENT.md`: packet/evidence/index/closure topology. No exception requested. |
| Verification | Build and run focused core input-source, VM host-ingress, KBC/native-device, Model-339 composition and affected DOS/guest input regressions where their declared external assets are available; run documentation governance. Static sweep must show platform/integration tests no longer access `vm_session` internals merely to inject host input, and no old ambiguous native-input symbol remains. |
| Expected Markers | Existing focused markers remain valid; add `M5:T375:S9:INPUT-BOUNDARY:OK` only if a new dedicated proof is needed. |
| Asset Needs | None. No ROM, guest media, source import, reference emulator, or external binary is admitted. |
| Reporting Requirements | Confirm the scope before implementation; report after the ingress/caller sweep and before the complete P; final delivery links evidence, exact focused result, commit, and remaining timing transfer. |
| Stop Conditions | Stop for owner direction if preserving host behavior requires a test-only API, a platform adapter cannot use the shared production ingress, a public ABI break cannot be contained to repository callers, or the change would alter keyboard/AUX protocol or timing. |
| Exit Criteria | One explicit production host-input operation is usable by host adapters and end-to-end tests; core native keyboard/mouse operations name attached-device ingress rather than generic injection; tests select the correct layer; no test-only API or duplicate guest-input path exists; focused proof and governance pass; evidence truthfully retains timing as open. |
| Original Owner Request | "Change the direct-injection API into explicit test/native-input boundaries; preferably do not create a test-only API; determine the right design." |
| Similar-Issue Sweep | Sweep every `core_machine_keyboard_submit_native*`, `core_machine_mouse_submit_relative`, `core_platform_input_source_submit`, `session->input_source`, `session->core_machine`, platform key/mouse adapter and VDM input caller. Classify each as host ingress, attached-device ingress, guest observation, or invalid bypass. |

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
