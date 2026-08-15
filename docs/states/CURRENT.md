# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T375 S13. |
| Admission And Approval | The owner approved continuing T375 toward the M5 L3 program and explicitly permits local 86Box/ROM research outside Git. The 2026-08-15 owner confirmation `approved always` covers this bounded, non-distributable reference experiment. No third-party code, firmware, media, trace, path, hash catalogue, or runtime dependency may enter NXVM. |
| Objective | Establish and execute, if the qualified local reference can be made reproducible without modifying NXVM's product boundary, the first project-owned 86Box CGA phase-measurement contract for the selected IBM 1504910-compatible Model-339 option. Record the reference identity, exact CRTC stimulus, checkpoints, comparison mask, timeout/no-progress budget, and result disposition. The S may admit only a source-labelled logical-phase refinement proven by that experiment; otherwise it records a precise transfer. |
| Non-goals | No whole-machine boot requirement, no 86Box/MAME/PCjs code import or product dependency, no ROM/guest-media commit or catalogue, no generic-AT scalar, no factory/default CRTC-table claim, no chosen CGA PCB/monitor/composite/snow assertion, no ISA contention/wait or monitor waveform model, no test-only production API, and no Model-339 or M5 L3 completion claim. |
| Reference Baseline | T375 S11's CGA-only non-interlaced 6845 logical state and S12's IBM feature-4910 / part-1504910 identity ledger. The qualified secondary reference is read-only local 86Box source revision `4fef696a`; its standalone IBM CGA model can cross-check a project-owned experiment but cannot establish a Model-339 board fact. The IBM adapter/manual sources listed in S12 remain primary for register semantics. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md). |
| Files And ABI Surface | Expected: `docs/states/CURRENT.md`, a new indexed evidence record under `docs/etc/evidence/`, `docs/etc/README.md`, `src/vm/profile/default_profile/pc_at_profile.c`, and `tests/machine/vm_model_339_clock_contract_smoke.c`; `src/core/machine/vadp.c` is eligible only if the qualified comparison proves an existing-owner correction. No public ABI or test-only API change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: one active packet, owner approval, P1/push/review/P2 closure and actual-change review. `docs/rules/ARCHITECTURE.md`: VADP remains the sole mutable CGA phase owner and external research cannot become a dependency. `docs/rules/CODING.md`: no test-only contract or duplicate timer. `docs/rules/DOCUMENT.md`: Current is the sole active contract and evidence is indexed. `docs/etc/operations/policy/source-policy.md`: local third-party/firmware research stays external and conclusions are neutral capability statements. |
| Verification | Confirm the reference binary/source provenance is reproducible before accepting an observation. Use a project-owned CRTC program that writes R0--R7/R9 through the existing register boundary; sample `3DAh` bit 0 and bit 3 at named character-clock checkpoints for one frame; bound each run to 30 seconds wall-clock and 2 frame periods of no-progress; compare only CRTC acceptance, logical display/blank partition, and vertical-sync edges. Replay NXVM's existing VADP CRTC/status tests and the adjacent EGA tests after any admitted correction. Run documentation governance before closure. |
| Expected Markers | `M5:T375:S13:MODEL339-CGA-REFERENCE-CONTRACT:OK`; if a runtime correction is admitted, retain the existing focused VADP markers rather than creating a test-only product surface. |
| Asset Needs | Owner-managed 86Box source/binary and any IBM/other ROM remain external research inputs. Keep them outside Git; no machine-local path, raw firmware byte, guest-media, or vendor hash catalogue in repository evidence. |
| Reporting Requirements | Report the reference qualification result before implementation, report any evidence-supported owner-local correction, and deliver the pushed P1, verification/evidence link, and an explicit retained transfer. The coordinator must inspect actual changes before acceptance. |
| Stop Conditions | Stop and record a transfer if the binary cannot be matched to a qualified source revision, the experiment requires whole-machine boot or unbounded GUI/manual observation, its configuration implies an unselected hardware dimension, the result conflicts with primary register semantics, or a proposed correction would require third-party/ROM import, a new public/test API, or a physical-board claim. |
| Exit Criteria | An indexed, reproducible measurement contract and its qualification/disposition exist; either a bounded result has source-labelled focused proof through the existing VADP owner or the exact unavailable prerequisite and earliest receiver are recorded. No external asset or dependency enters NXVM, and applicable focused/documentation gates pass. |
| Original Owner Request | Continue the current task through comprehensive L3 closure before Windows 3.1; use 86Box/MAME/PCjs as secondary references when primary manuals lack deterministic timing; use bridge/differential experimentation rather than requiring a full reference-machine boot; do not create pure-test APIs. |
| Similar-Issue Sweep | Inspect every selected Model-339 CGA timing claim, the VADP CRTC/status owner, clock-plan publication, reference qualification record, and neighboring EGA route. Ensure no prior evidence turns a generic emulator assumption, local ROM observation, or synthetic default phase into a selected-board fact. |

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
