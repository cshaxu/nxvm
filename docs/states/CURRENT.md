# Project Status

## Current Work

## M5 T375 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: user. Approval: persistent instruction to continue implementing the current L3 plan, reaffirmed in this session; scope limited to the S5-selected composition virtual-time boundary. No source/media exception is needed. |
| Objective | Bind an injected, production VM-session virtual-time source to the Model-339 composition path, and use a nonzero source-tick batch only after the core reports `WAITING_FOR_INTERRUPT` while the runner remains active and not completing a single-step pause. |
| Non-goals | Do not derive guest time from host sleep, add a platform monotonic clock, claim timer/phase/Model-339 L3 closure, change CPU-retirement accounting, or add a test-only public injection API. |
| Reference Baseline | T375 S4 explicit core machine-time contract (`91539339`, accepted `2eedff53`) and T375 S5 source-policy selection (`5832334e`, accepted `d27b0e5a`). |
| Candidate Proposal | [M5 5170 board/phase timing closure](../proposals/m5-5170-board-phase-timing-closure.md); S5 policy evidence selects the session-owned injected provider contract. |
| Files And ABI Surface | `src/vm/composition/session/session_interface.h`, session storage, reset and runner paths, a composition-private virtual-time helper, focused product/composition smoke target, evidence index and CURRENT status only. The session configuration gains a production source descriptor; its caller-owned context must outlive the session. |
| Applicable Rules | Architecture layering: source selection remains VM composition-owned and calls the core only through `core_machine_advance_time`; coding/lifecycle rules require explicit ownership, reset and failure behavior; source policy forbids host sleep as time. Evidence records the exact call gates and proof. No exception requested. |
| Verification | Configure and build the focused smoke; prove a supplied nonzero batch reaches the core time API exactly once, zero/absent source does not advance, a reset invokes the provider rebase callback, and the runner-facing helper rejects a non-waiting result or inactive control state. Static runner review proves the single-step pause excludes source consumption. Run documentation governance. |
| Expected Markers | `M5:T375:S6:VIRTUAL-TIME-SOURCE:OK` and documentation-governance success. |
| Asset Needs | None. No ROM, guest medium, or host-duration source is used. |
| Reporting Requirements | Record source lifetime, activation/reset/failure semantics, exact focused proof and remaining host-source transfer in indexed evidence. Report the implementation P and later independent acceptance P concisely. |
| Stop Conditions | Stop for a required host-clock semantic decision, an ABI/lifetime conflict that cannot be bounded here, or a discovered need to change time authority beyond S5; otherwise transfer host source/rate policy to later S. |
| Exit Criteria | A caller can configure a non-test-only session source; only active, non-step `WAITING_FOR_INTERRUPT` processing can consume it; one nonzero source batch calls core explicit time once; reset rebases it; defaults remain non-advancing; focused proof and governance pass; no L3 claim is made. |
| Original Owner Request | Continue the current task plan toward L3, and specifically avoid pure test APIs when clarifying direct injection versus native input boundaries. |
| Similar-Issue Sweep | Inspect all session creation paths, reset paths and runner wait branches; retain no alternate direct time advancement or host-sleep-derived guest clock. |

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
| T375 S5 | Accepted P1 `5832334e`: Model-339 virtual source ticks must be an injected composition capability; host sleep is rejected as a timing source and S6 owns the binding/proof. [Policy](../etc/evidence/t375-s5-model339-virtual-time-policy.md). |
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
