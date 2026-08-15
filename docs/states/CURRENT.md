# Project Status

## Current Work

## M5 T373 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T373 remains the latest open numeric task and S4 follows accepted S3. |
| Admission And Approval | The owner approved the ordered three-baseline, functional-before-timing L3 program and the capability-ledger candidate now under closure review. |
| Objective | Perform T373's task-level closure audit: map every proposal requirement to S1--S3 evidence, confirm that all residual implementation work has an exact Queue/TODO receiver, and make a truthful close/not-close decision without converting a classified gap into an L3 claim. |
| Non-goals | No device/profile implementation, new source research, timing value, ROM/media use, Queue reordering, or Windows work. An unresolved requirement stops closure rather than being papered over. |
| Reference Baseline | [Capability-ledger proposal](../proposals/m5-baseline-device-capability-ledger.md); T373 S1 support inventory; T373 S2 BOM/source ledger; T373 S3 capability classification; M5 Queue and TODO. |
| Candidate Proposal | [Baseline-machine and supported-device capability ledger](../proposals/m5-baseline-device-capability-ledger.md). |
| Files And ABI Surface | New T373 S4 closure-audit evidence, history record, `docs/etc/README.md`, and `docs/states/CURRENT.md` only; no source, ABI, profile, build, ROM, media, Queue, or TODO mutation in P1. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` task closure, evidence, P lifecycle and role review; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; source policy's external-asset boundary. |
| Verification | Inspect the proposal, all T373 evidence, current Queue and TODO receivers, and the actual P1 document changes; create a requirement-to-evidence/transfer matrix; run documentation governance and `git diff --check`. Coordinator P2 may close only if every in-scope gap is assigned and the final record remains non-L3. |
| Expected Markers | One indexed closure audit answers every required-scope and evidence-standard clause, proves that no capability is silently unowned, and makes T373's result an explicit planning prerequisite rather than a machine-completion claim. |
| Asset Needs | None. No ROM/media/download/reference runtime is required or admitted. |
| Reporting Requirements | Report closure audit decision, exact accepted evidence, residual receivers, focused verification, pushed P1, coordinator acceptance/rejection, and the next candidate eligible for admission. |
| Stop Conditions | Do not close if a requirement lacks evidence, a selected/public capability lacks a functional or timing receiver, a proposal/Queue link is stale, or an audit finding would require implementation. Transfer an implementation finding to its established receiver. |
| Exit Criteria | Every T373 proposal clause maps to durable S1--S4 evidence and exact later receiver where work remains; no DeskPro/XT unknown has become inferred hardware; no current-product capability lacks a disposition; governance checks pass; the coordinator can truthfully close T373. |
| Original Owner Request | Build the three-machine capability/deficit ledger first, then implement functional and L3 closure in Queue order before Windows 3.1. |
| Similar-Issue Sweep | Recheck every classified selected baseline and current-product row against Queue/TODO: 5170 selected devices, DeskPro/XT profile composition, EGA/ATA/AUX, unsupported peripherals, external ROM/media, and MFM exclusion; each must have one stated disposition and no duplicate receiver. |

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
| T373 S3 | P1 `dbeeeb0b` reconciles the three-machine BOM and current product support into classification, concrete gaps and exact functional/timing receivers. No baseline capability is complete; a task-level closure audit remains. [Capability ledger](../etc/evidence/t373-s3-baseline-capability-classification.md). |
| T372 | Closed at `3f56c72c`: S1--S8 establish that Model 339 has selected logical ownership and deterministic ordering, but lacks selected-device functional closure and source-backed board/phase timing. **5170 model-L3 is not ready.** [Closure audit](../etc/evidence/t372-s8-task-closure-transfer-audit.md). |
| T370 | Closed at `77a73c04`: S1--S5 reconciled all selected Model-339 device-service owners and transferred unavailable duration to phase refinement; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t370-s5-planar-cga-transfer-audit.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |
| T364 | Closed at `7d574ae3`: all selected PC/AT components, ports/routes and lifecycle/timing owners are inventoried; optional and physical gaps retain exact Queue/TODO receivers. [Closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md). No model-L3 claim. |

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
