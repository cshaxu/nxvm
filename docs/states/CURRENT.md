# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation - M5 T375 S16. |
| Admission And Approval | Owner approval is the user's continuing instruction to implement the active L3 plan, including the explicit approval to use 86Box as a bridge/differential reference without requiring a whole-machine guest. This S uses only an external, non-product 86Box v6.0 source/build and creates no ROM, media, source, or runtime dependency. |
| Objective | Produce a bounded, reproducible 86Box v6.0 PIT/PIC IRQ0 reference trace for the selected 8 MHz Model-339 timing receiver, then compare its declared event checkpoints against NXVM's existing PIT/PIC proof and record either a concrete receiver correction or a truthful no-correction transfer. |
| Non-goals | Do not boot a guest operating system, claim an IBM physical waveform, alter a public/runtime API, import or derive third-party source, commit firmware/media, or close 5170 model-L3. This is not a generic 86Box integration. |
| Reference Baseline | T375 S13's pinned 86Box v6.0 source revision `4fef696a`, T375 S14 CGA phase crosswalk, selected Model-339 clock plan, and NXVM's T350 S2 PIT IRQ0 proof. Intel/IBM authorities remain controlling; this same-profile external model is a secondary behaviour reference where those authorities do not fix an ordering/cadence value. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Repository surface is limited to this packet and one indexed evidence record; a production code/test correction is permitted only if the bridge demonstrates a concrete in-scope PIT/PIC ordering defect. External build, logs, temporary instrumentation, and raw traces remain below `O:\repos.external\86box-source\build\nxvm-pit-bridge` and are not committed. No ABI changes are allowed. |
| Applicable Rules | Execution packet/P1-P2/review/cleanup requirements apply. Architecture ownership and no-new-dependency invariants apply; existing core PIT/PIC owners remain the sole NXVM truth. Coding test-boundary and no-test-only-public-API rules apply to any correction. Source policy permits external research but prohibits source/firmware/media import or product dependency. Documentation rules require indexed supporting evidence and compact Current status. |
| Verification | Configure and build the external reference with PIT/PIC logging enabled; run one bounded, no-guest IRQ0 stimulus with a named input/event schema, 30-second wall-clock and 1 MiB raw-log budget; retain only normalized checkpoints after deleting raw logs. Compare reload/program, OUT transition, PIC raise, acknowledge/EOI ordering and bounded cadence against the existing NXVM PIT/PIC focused test. Run its existing focused test and the documentation governance gate for repository changes. |
| Expected Markers | External reference emits a bounded PIT/PIC trace and the evidence records `M5:T375:S16:PIT-PIC-BRIDGE:OK`; NXVM focused proof retains `M5:T350:S2:PIT-IRQ0:OK` or its currently registered equivalent. |
| Asset Needs | The owner-approved, external IBM 5170 Rev.3 ROM set already held under `O:\assets\bios` or the owner-local 86Box ROM bundle may initialize the reference only; no guest disk/media is mounted. It remains outside Git and is neither catalogued nor distributed by NXVM. The external 86Box source/build dependencies and transient trace are owner-local research inputs; all raw logs are deleted after checkpoint extraction. |
| Reporting Requirements | Report the reference revision, input schema, checkpoints, comparison result, cleanup result, any NXVM correction, focused verification, and the remaining board/device receivers. Do not represent a model trace as physical IBM measurement. |
| Stop Conditions | Stop and transfer if the reference cannot be built with available lawful local dependencies, if obtaining a dependency requires importing protected media/source into NXVM, if a comparison requires a whole guest or unbounded recorder, or if the reference result conflicts with controlling Intel/IBM material. |
| Exit Criteria | A reproducible bounded reference trace and normalized comparison evidence exist; any demonstrated NXVM defect is repaired with focused proof, otherwise no correction is claimed; external raw artifacts are removed; P1 is reviewed and a P2 status-only closure records the exact transfer. |
| Original Owner Request | Continue the L3 implementation; 86Box/MAME/PCjs may supply deterministic timing only when primary manuals do not; bridge/differential use is acceptable and does not require running a whole machine. |
| Similar-Issue Sweep | The potential defect class is PIT/PIC event ordering/cadence. Inspect the selected PC/AT callback/arbitration path and its focused PIT/PIC tests plus existing timing evidence. Any hit outside this narrow receiver is deferred to its earliest owner rather than repaired through a second timing path. |

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
