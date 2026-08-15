# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T375 S14. |
| Admission And Approval | The owner approved continued T375 execution toward L3, authorized local 86Box reference research outside Git, and reaffirmed standing approval on 2026-08-15. This S admits only an owner-local audit and, if warranted, an existing-VADP-owner correction; external source, firmware, media, path, trace, and dependency containment remain mandatory. |
| Objective | Reconcile the qualified 86Box-v6.0 IBM-CGA phase-state model against the primary MC6845/IBM logical CRTC contract already implemented by VADP. Produce a register/phase/status crosswalk and either repair one evidence-supported VADP logical-state defect or record that no correction is admitted. |
| Non-goals | No whole-machine boot, reference-binary trace, code/ROM import, default CRTC programming, generic-AT change, selected-board cadence or physical waveform assertion, ISA contention/waits/snow, light-pen completion, R8 interlace/skew support, public or test-only API, or 5170/M5 L3 claim. |
| Reference Baseline | T375 S11 owns primary-source non-interlaced MC6845 CRTC logic; T375 S12/S13 qualify 86Box v6.0 only as a constrained secondary IBM-CGA cross-check and bind its character cadence solely to the selected VADP clock. IBM CGA `3DAh` bit meanings remain primary. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Expected: `docs/states/CURRENT.md`, a new indexed evidence record under `docs/etc/evidence/`, `docs/etc/README.md`, and only if the crosswalk finds a bounded defect, `src/core/machine/vadp.c` plus its existing focused VADP test. No public ABI or new test-only capability. |
| Applicable Rules | `docs/rules/EXECUTION.md`: one active packet, review of actual changes, sequential pushed P1 and P2. `docs/rules/ARCHITECTURE.md`: VADP remains sole mutable CGA-phase owner; research is not a dependency. `docs/rules/CODING.md`: no duplicate timer or test-only contract. `docs/rules/DOCUMENT.md`: Current is the only active contract and evidence is indexed. `docs/etc/operations/policy/source-policy.md`: external source stays outside NXVM and only neutral conclusions enter evidence. |
| Verification | Compare CRTC write masks/readback, programmed/unprogrammed phase selection, displayed/horizontal-blank partition, vertical-sync transition, reset/restart and mode-control gating against primary requirements and constrained 86Box behavior. If corrected, replay `core-machine-vadp-text-status-smoke`, Model-339 clock smoke, and both adjacent EGA smokes; run documentation governance. |
| Expected Markers | `M5:T375:S14:MODEL339-CGA-PHASE-CROSSWALK:OK`; retain existing VADP markers for any correction. |
| Asset Needs | Read-only owner-managed 86Box v6.0 source only. No external binary output, ROM, media, asset path, hash catalogue, code, or trace is committed. |
| Reporting Requirements | State the crosswalk disposition before a correction, deliver any pushed P1 with focused proof and indexed evidence, then independently inspect actual changes before P2 acceptance. State retained timing/L3 transfers explicitly. |
| Stop Conditions | Stop with a transfer if the reference differs only in an unselected adapter option or physical model, conflicts with primary logical requirements, requires an external runtime trace, expands into default firmware programming or physical cadence, or would introduce an external dependency/test-only API. |
| Exit Criteria | Indexed crosswalk completes every selected phase/status route; any admitted correction is owner-local with focused proof, otherwise the no-correction disposition and earliest receiver are explicit. No external asset/dependency enters NXVM and documentation governance passes. |
| Original Owner Request | Continue current work toward complete L3 before Windows 3.1; use 86Box/MAME/PCjs as secondary references when primary manuals lack deterministic timing; bridge/differential work need not boot a full reference machine; no pure test API. |
| Similar-Issue Sweep | Sweep all VADP CRTC writes/masks/readback, reset/advance/status/snapshot paths, Model-339 clock publication and adjacent EGA CRTC gates, ensuring no reference-specific physical assumption becomes a second phase owner. |

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
