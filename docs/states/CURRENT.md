# Project Status

## Current Work

## M5 T370 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T370 S1 accepted at `990e31a3`. |
| Admission And Approval | The owner-approved ordered L3 program continues. S1 authorizes a PIT/PIC/RTC source-and-domain reconciliation only; it does not authorize a timing value, clock conversion or device behavior change. |
| Objective | Reconcile the selected PC/AT PIT IRQ0, dual-PIC logical acknowledgement and RTC/CMOS IRQ8 command/event contracts with current clock-domain, callback, trace, reset and copied-consumer routes; determine exactly which source facts can be represented now and which require a later conversion/model. |
| Non-goals | No runtime timing scalar, PIT mode expansion, physical INTA, host time, RTC calendar change, new trace ABI, DMA/FDC/KBC/VADP work, ATA/HDC/MFM route, phase refinement or 5170-L3 claim. |
| Reference Baseline | T370 S1 ledger; T349 PIC, T350 PIT/RTC and T369 bus-stage evidence; Intel 8254/8259A, Motorola MC146818A and IBM PC/AT technical references; current `pit.c`, `pic.c`, `rtc.c`, `machine.c` and focused owner tests. |
| Candidate Proposal | [PC/AT device service-timing corpus](../proposals/m5-device-service-timing-corpus.md). |
| Files And ABI Surface | S2 evidence, T370 history, Current status and evidence index only. Stop and revise before any source, ABI, CMake or timing-table edit. |
| Applicable Rules | One-owner device lifecycle; validation-before-publication; reset/cancel and copied-consumer boundaries; source-policy and documentation rules. The deterministic timeline is not itself a physical time source. |
| Verification | Primary-source fact table, production/test caller sweep, existing PIT/PIC/RTC smoke inventory, documentation governance and diff check. No task artifact or runtime marker is manufactured by this reconciliation-only S. |
| Expected Markers | Existing device smokes remain evidence; S2 introduces no new marker and does not claim a frequency conversion. |
| Asset Needs | No ROM, guest media, firmware, external code, binary, raw trace or host observation. Public manuals and project-authored evidence only. |
| Reporting Requirements | Separate chip input/behavior facts from board wiring and project-domain conversion; map every event/ack/reset route; record reference-model exclusion or cross-check limit; name the exact S3 receiver. |
| Stop Conditions | Stop and revise if a source fact requires a guessed tick ratio, host pacing, physical INTA assertion, changed device behavior, a second scheduler or a public interface. |
| Exit Criteria | P1 commits an indexed PIT/PIC/RTC source-to-route/domain reconciliation with a bounded S3 receiver; it preserves all existing behavior and leaves 5170-L3 open. |
| Original Owner Request | Continue in Queue order toward complete L3 before Windows 3.1; use named reference models only where authoritative manuals leave a range or no range, without treating them as IBM authority. |
| Similar-Issue Sweep | Sweep PIT output/ports/reset, PIC source/select/ack/refresh/reset, RTC port/register-C/event/reset, machine arbitration/readiness callbacks, clock-domain configuration, trace consumers, selected Model-339 composition and all focused device tests; classify each hit or revise the packet. |

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
| T370 S1 | P1 `a3df36dd` records the complete selected-Model-339 device service owner/source/lifecycle ledger and bounded S2--S5 sequence. It allocates no timing scalar, device behavior, artifact or 5170-L3 claim. [S1 ledger](../etc/evidence/t370-s1-pcat-device-service-owner-ledger.md). |
| T369 | Closed at `08a64bea`: S1--S4 lock and audit the 5170 Model-339 bus-stage input, logical 80286 DMA handoff and selected FDC/PIC visibility. Board waits, physical waveforms, device service, phase refinement and final 5170 L3 remain exact ordered transfers; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t369-s4-pcat-bus-stage-closure-audit.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |
| T364 | Closed at `7d574ae3`: all selected PC/AT components, ports/routes and lifecycle/timing owners are inventoried; optional and physical gaps retain exact Queue/TODO receivers. [Closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md). No model-L3 claim. |
| T363 | Closed at `312ef2f9`: all 256 primary and 256 secondary dispatch slots have one source-backed successful-retirement owner or an exact explicit receiver; inventory and 246/246 current-gate passed. [Closure audit](../etc/evidence/t363-s7-complete-instruction-timing-closure-audit.md). No physical/cycle-exact L3 claim. |
| T362 | Closed at `316c2334`: S1--S2 normalize every eligible 80186 immediate-IMUL source row in one private direct/constrained model table; S3 resolves stale full-gate verifier assumptions and records 246/246 current-gate tests. [Closure audit](../etc/evidence/t362-s3-legacy-timing-normalization-closure-audit.md). Model-L3 only; no physical/cycle-exact claim. |

## Recent Governance

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

- **M5 Td S87:** reconciled the T358 continuation handoff: removed the
  duplicate empty history heading, retained the approved Queue order, and
  restored compact open-T358 status. Documentation governance and diff check
  passed; Td work has no runtime or artifact change.

- **M5 Td S87:** reordered M5 around four-profile timing authority,
  device/chip/port/bus completeness, bus availability, service timing,
  cycle-exact selected-profile work, and L3 closure before final BYOB Windows
  3.1 Standard/386 Enhanced lifecycle proof. M6 now follows the M5 closure
  decision. T358 S1 was isolated during this Td delivery and is now restored
  as the sole active packet. Documentation governance and diff check passed;
  Td work has no runtime change.
