# Project Status

## Current Work

## M5 T375 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner approved continuing the ordered M5 program through complete L3 closure, with T375 next after T374; approval is retained in this conversation on 2026-08-15. The owner further requires 86Box/MAME/PCjs only as cross-checks where IBM/Intel authority gives a range or no timing, never as precise-machine authority; raw `.img` remains the sole admitted floppy format, with 765 Deleted/Control-Mark and Scan explicitly deferred. |
| Objective | T375 is the next approved Queue candidate; S1 is its bounded timing-ledger and production-route audit before any timing repair. Produce a source-labelled Model-339 timing inventory that maps every T375 selected CPU/bus, DMA/HOLD, PIC/PIT/RTC, 8042/keyboard, FDC/floppy, CGA, NMI/reset and cross-device phase boundary to its sole production owner, primary-source/probe requirement, current evidence, and exact/range/reference-exhausted disposition. Select no scalar without that basis. |
| Non-goals | No functional repair, generic-PC/AT substitution, fixed-disk/MFM or ATA route, factory claim for the aftermarket 1.44 MB drive, physical waveform/cycle-exact claim, host-time coupling, ROM/media import, reference-emulator authority, final Model-339 L3 decision, or closure of raw-IMG 765 Deleted/Control-Mark/Scan debt. |
| Reference Baseline | IBM PC/AT 5170 Model 339 / Type 3, 8 MHz 80286, Rev.3 BIOS slot, 512 KB planar memory, 101-key AT keyboard, CGA, no fixed disk, planar-parity NMI, and aftermarket-only 1.44 MB drive. T368 closes only successful-retirement CPU rows; T369/T370/T372 and T374 close logical/function scopes and transfer board/device phase timing. |
| Candidate Proposal | [IBM PC/AT 5170 board and device phase-timing closure](../proposals/m5-5170-board-phase-timing-closure.md); Queue position 1, preceding `m5-5170-final-l3-audit.md`. |
| Files And ABI Surface | Audit `src/core/machine/{machine,transaction,dma,pic,pit,rtc,kbc,fdc,vadp}.{c,h}`, relevant VM Model-339 profile/composition routes, and focused tests/targets. S1 changes only durable evidence/status registration if required; it introduces no public ABI or production timing behavior. |
| Applicable Rules | `docs/README.md` Task Reading Set; `CONTRIBUTING.md`; `docs/rules/EXECUTION.md` S/P lifecycle, source/evidence and closure rules; `docs/rules/DOCUMENT.md`; `docs/etc/operations/policy/source-policy.md`; `docs/design/{ROADMAP,ARCHITECTURE,CODING}.md`; `docs/rules/{ARCHITECTURE,CODING}.md`. Preserve one owner, one production transaction path, opaque public boundaries, source provenance, and no test-only public route. |
| Verification | Complete route/search sweep; inspect each selected owner and relevant focused test; verify all claimed supporting documents are source-labelled; run documentation governance and `git diff --check`. If S1 adds no code, no full-build claim is required; known stale whole-build fixtures remain separately recorded. |
| Expected Markers | `M5:T375:S1:MODEL339-TIMING-INVENTORY:OK` in durable evidence after reconciliation; any replayed existing markers must be reported exactly and not upgraded into timing proof. |
| Asset Needs | Primary IBM PC/AT Technical Reference and Intel/component documentation may be consulted externally under source policy; local owner-managed ROMs/media remain external and uncommitted. 86Box/MAME/PCjs are permitted only named behaviour cross-checks after the primary-source disposition. |
| Reporting Requirements | Confirm or raise a material objection before execution; report progress after route inventory and source-disposition sweep; final delivery links durable evidence, exact verification, every transfer, and states explicitly that Model-339 L3 remains open. |
| Stop Conditions | Stop and return a finding to the earliest functional owner if an incomplete selected state/IRQ/DMA/reset/consumer route is discovered. Stop without a scalar when primary sources/probes do not support one; record range or reference-exhausted transfer. Pause for owner direction if a new asset format, ROM/media import, baseline change, or non-T375 capability is required. |
| Exit Criteria | A complete selected-board timing ledger exists with route/owner/source/disposition for every required T375 boundary; functional defects, source-exhausted rows and final-audit inputs have named receivers; documentation verification passes; no Model-L3/cycle-exact claim is made. |
| Original Owner Request | Owner requested that direct injection become an explicit test/native-input boundary, preferably with no test-only API; subsequent standing instruction: continue the approved task sequence until comprehensive L3 closure, with 86Box/MAME/PCjs as secondary cross-checks only, no ROM/media commits, and raw IMG-only floppy scope. |
| Similar-Issue Sweep | Search all selected machine/controller timing publication, timeline/refresh/arbitration, test injection and profile routes for duplicate clocks, host-time coupling, generic-PC/AT fallback, unowned waits, stale direct keyboard injectors and consumer-specific timing. Classify every result as selected, excluded, or an explicit receiver. |

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
