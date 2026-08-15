# Project Status

## Current Work

## M5 T374 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T374 remains the latest open numeric task and S3 follows accepted S2. |
| Admission And Approval | The owner approved the Model-339 primary-source 8042 pulse-reset repair selected by T374 S2, before board/device timing closure. |
| Objective | Implement IBM 8042 command-port `F0h`--`FFh` output-port pulse semantics at the existing KBC owner: a command with bit 0 clear requests the existing CPU reset operation while preserving persistent output-port/A20 state; a bit-0-set command does not request reset. |
| Non-goals | No pulse duration conversion, timing scalar, new reset vector policy, controller rewrite, external ROM/media use, AUX change, EGA/VGA, ATA/HDC, MFM/ST-506, generic PC/AT expansion, or Model-339 L3 claim. Other output-port pulse bits remain unobservable/no-op. |
| Reference Baseline | T374 S2 functional contract; IBM PC/AT Technical Reference March 1986 (F0h--FFh pulse-output-port contract); `core_machine_kbc_apply_output_port()` existing D1h reset/A20 owner; T374 S1 focused test boundary. |
| Candidate Proposal | [IBM PC/AT 5170 selected-device functional closure](../proposals/m5-5170-selected-device-functional-closure.md). |
| Files And ABI Surface | `src/core/machine/kbc.c`, cohesive KBC smoke, T374 evidence/history/index, and `docs/states/CURRENT.md`; no public header/ABI, profile descriptor, media, firmware or runtime configuration change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` implementation/P lifecycle, evidence and similar-issue rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; `docs/design/ARCHITECTURE.md`; `docs/design/CODING.md`; `docs/rules/ARCHITECTURE.md`; `docs/rules/CODING.md`; source policy. |
| Verification | Add a focused KBC regression that proves FEh requests reset once, FFh requests none, output-port/A20 persist unchanged, and D0h/D1h plus existing keyboard/AUX behavior remain. Fresh-clean/rebuild the changed KBC target and selected Model-339 composition smoke through Git-Bash Ninja; run both and documentation governance plus `git diff --check`. |
| Expected Markers | One owner-local command decode routes reset only through the existing KBC-to-execution boundary, with no duplicate reset path and no persistent mutation from a pulse command. |
| Asset Needs | None. Primary documentation was read only; no ROM, firmware, media, trace, source import or local asset is added. |
| Reporting Requirements | Report owner-boundary confirmation, repair/test result, similar-command sweep, fresh build/replay, pushed P1, coordinator acceptance and next Model-339 functional receiver. |
| Stop Conditions | Stop if implementing pulse behavior requires a project duration, a reset-vector/triple-fault policy, a new board signal consumer, a generic device capability, or a primary-source conflict; transfer rather than infer. |
| Exit Criteria | `F0h`--`FFh` command decoding satisfies the selected bit-0 reset/no-reset contract through the existing owner; no persistent A20/output-port mutation occurs; focused regression and fresh target rebuild pass; all non-goals remain intact. |
| Original Owner Request | Implement the selected machine's functional gaps before timing/L3 closure, treating exact documentation as authority and reference emulators only as fallback. |
| Similar-Issue Sweep | Inspect all command-port cases and every caller of `core_machine_kbc_apply_output_port()`/CPU reset request; verify that D1h retains its persistent-output contract, pulse commands gain only transient reset delivery, and no AUX/platform/firmware path duplicates the reset request. |

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
| T374 S2 | P1 `e6b6bcc5` establishes the Model-339 primary functional contract and selects the 8042 F0h--FFh pulse-reset repair. The pulse duration is explicitly later timing work. [Contract matrix](../etc/evidence/t374-s2-model339-functional-contract-matrix.md). |
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
