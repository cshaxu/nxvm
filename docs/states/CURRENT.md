# Project Status

## Current Work

## M5 T374 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T374 remains the latest open numeric task and S6 follows accepted S5. |
| Admission And Approval | The owner approved completing Model-339 selected-device functional gaps before timing/L3 closure and authorized continuing T374. S4/S5 establish the FDC controller contract and its ready-attention owner. |
| Objective | Reconcile the complete 8272A/uPD765 command table against the current FDC dispatcher, selected Model-339 compatibility boundary and generic media contract; select the earliest source-backed repair or explicit prerequisite sequence for every absent or over-admitted command. |
| Non-goals | No command implementation in this audit; no controller/drive timing, rotation, flux/index model, DMA service timing, board wait, media import, ROM, ATA/HDC, MFM/ST-506, EGA/VGA, AUX, generic-product support expansion, or Model-339 L3 claim. |
| Reference Baseline | T374 S4/S5 FDC evidence; Intel 8272A and NEC uPD765 primary command tables; `core_machine_fdc_command_length()` and `core_machine_fdc_execute()`; core media provider capability/operation boundary; selected 1.44 MB compatibility remains aftermarket only. |
| Candidate Proposal | [IBM PC/AT 5170 selected-device functional closure](../proposals/m5-5170-selected-device-functional-closure.md). |
| Files And ABI Surface | Evidence/history/index and `docs/states/CURRENT.md` only for the audit P; FDC, media and profile code/tests are read-only inspection surfaces. A separately admitted repair packet names implementation files and ABI. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` implementation/P lifecycle, evidence and similar-issue rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; source policy. Architecture/coding authorities govern a later code change, not this documentation-only audit. |
| Verification | Produce a command-table-to-dispatcher/media-capability matrix; inspect every `core_machine_fdc_command_length()`/execute case, invalid fallback, FDC config construction and relevant media operations; run unchanged focused FDC/Model-339 topology tests through Git-Bash CMake/Ninja; run documentation governance and `git diff --check`. |
| Expected Markers | A source-labelled classification for every 8272A/uPD765 command: complete, source-compatible invalid, missing owner-local behavior, or blocked by an explicit media/profile prerequisite. No enhanced-controller command is silently attributed to Model 339. |
| Asset Needs | None. Controller manuals are read only; no ROM, firmware, guest media, trace, external implementation or local asset is added. |
| Reporting Requirements | Report command/owner disposition, profile-identity finding, selected repair or transfer, focused baseline replay, pushed P1, coordinator acceptance and next functional receiver. |
| Stop Conditions | Stop and transfer if selected hardware cannot be identified to controller-command level, a command requires unadmitted media metadata/operation or profile-capability design, or source conflict remains. Do not implement a guessed subset. |
| Exit Criteria | Evidence classifies every command and selects only an evidence-backed, bounded next receiver; unchanged focused regressions pass; no command/timing/L3 overclaim remains. |
| Original Owner Request | Implement each selected machine's functional gaps before timing/L3 closure, treating exact documentation as authority and reference emulators only as fallback; use CMake through Git Bash for local builds. |
| Similar-Issue Sweep | Compare all command opcodes, modifier-bit handling, result/IRQ/DRQ/reset paths, current `VERSION` behavior, every media provider operation/capability, descriptor bindings and tests so unsupported controller generation or command semantics cannot remain implicit. |

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
| T374 S5 | P1 `60a772ff` adds source-backed FDC Ready-transition attention through existing IRQ6/Sense-Interrupt ownership, distinct from DIR Disk Change. FDC commands plus controller/drive/board timing remain open. [Repair evidence](../etc/evidence/t374-s5-fdc-ready-attention-repair.md). |
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
