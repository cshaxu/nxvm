# Project Status

## Current Work

## M5 T374 S11 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T374 S10 is accepted and remains the sole compact progress row. |
| Admission And Approval | The owner approved continued T374 selected-device functional closure before timing/L3, including the explicit S10 raw-IMG boundary. The T374 proposal requires functional closure of selected CGA before board timing. |
| Objective | Audit the complete retained digital-CGA functional surface of the selected Model 339: mode/color/CRTC/status ports, B8000h layout, reset, snapshot and INT 10h consumer behavior; identify one concrete, evidence-backed next repair or explicit transfer. |
| Non-goals | No timing/cadence/contention values, composite output, EGA/VGA behavior, external ROM or guest-media use, host renderer change, 765 work, or Model-339 L3 claim. No code change occurs in this audit. |
| Reference Baseline | T374 S1 owner inventory; T366 S6 Model-339 CGA topology; `vadp.{h,c}`; retained CGA and VM system smoke routes; IBM CGA/PC-AT technical documentation as primary behavior authority. |
| Candidate Proposal | [IBM PC/AT 5170 selected-device functional closure](../proposals/m5-5170-selected-device-functional-closure.md). |
| Files And ABI Surface | Evidence/history/index/status only for this audit; read-only inspection of core VADP, profile composition, generated firmware INT 10h path and CGA/Model-339 tests. A later repair names its exact source and snapshot ABI surface. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` lifecycle/evidence/similar-issue rules; `docs/rules/DOCUMENT.md`; `CONTRIBUTING.md`; source policy. Architecture/coding authorities apply to a later code repair, not this audit. |
| Verification | Build a port/register/mode/reset/snapshot/consumer matrix; sweep VADP production routes and all CGA tests; run unchanged core-CGA and Model-339 focused targets through Git-Bash CMake/Ninja; run documentation governance and `git diff --check`. |
| Expected Markers | Every retained Model-339 CGA surface is classified present, partial, absent or excluded with one owner and test route; any functional gap has one earliest receiver and no timing value or EGA/VGA implication. |
| Asset Needs | None. Documentation is read only; no ROM, firmware, guest media, source import or local asset is opened, copied, written or committed. |
| Reporting Requirements | Report the full retained-CGA matrix, exact primary-source boundary, complete production/test sweep, focused baseline, pushed P1, coordinator acceptance and next repair/transfer. |
| Stop Conditions | Stop and transfer if a functional conclusion requires unapproved physical video timing, composite modeling, a vendor ROM, protected media, or a public display ABI change beyond a bounded repair. |
| Exit Criteria | Indexed evidence classifies all retained Model-339 CGA functional routes and selects exactly one bounded repair or explicit transfer; focused baseline and documentation checks pass; no code, timing or L3 claim is introduced. |
| Original Owner Request | Fully implement selected-machine functional gaps before timing/L3, retain external media/ROMs outside Git, and continue toward the pre-Windows L3 program. |
| Similar-Issue Sweep | Search all tracked VADP ports, CGA mode/color/status/CRTC handling, VRAM layout, reset, snapshot, profile composition, INT 10h and CGA test routes so one mode or consumer cannot be silently excluded. |

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
| T374 S10 | P1 `3a583092` records the owner-selected raw-IMG-only floppy boundary and all persistence routes; Deleted/Control-Mark and separately Scan command work remain explicit TODOs, while ordinary sector behavior is unchanged. [Boundary evidence](../etc/evidence/t374-s10-raw-img-address-mark-boundary.md). |
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
