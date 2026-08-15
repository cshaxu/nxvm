# Project Status

## Current Work

## M5 T369 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T369 S1 accepted at `2ca0c00a`. |
| Admission And Approval | The owner-approved T369 bus-timed PC/AT package continues under the standing before-Windows L3 objective. S1's evidence permits only primary-board/CPU contract reconciliation; it did not approve a timing scalar or a secondary-model substitution. |
| Objective | Recover and reconcile the selected IBM 5170 board's documented READY/HOLD/HLDA/DMA signal facts with Intel 80286 CPU semantics, then decide whether one source-labelled shared availability contract is implementable or must remain explicitly transferred. |
| Non-goals | No guessed wait count, elapsed-clock conversion, copied reference-model code, ROM/media use, ATA/HDC admission, controller-service duration, physical waveform, resumable CPU rewrite, or 5170-L3 claim. |
| Reference Baseline | T369 S1 ledger at `87460471`; T366 Model 339 lock; IBM September 1985/March 1986 technical references and IBM board diagrams; Intel 210760-002 80286 Hardware Reference; source policy. 86Box/MAME/PCjs may check an already-qualified fact only. |
| Candidate Proposal | [Bus-timed PC/AT operation](../proposals/m5-bus-timed-pcat-operation.md). |
| Files And ABI Surface | S2 evidence, T369 history and active packet/index only unless the sources prove a bounded shared contract. Any runtime change requires a packet revision naming `transaction.*`, `machine.c`, profile composition and focused tests before implementation. |
| Applicable Rules | Execution evidence/objection/change-control rules; documentation boundaries; architecture one-owner, validation-to-publication and composition invariants; source-policy research/no-import rules. Unqualified timing sources are not implementation authority. |
| Verification | Record exact document/page-or-section citations and a signal-to-current-route matrix; cross-check any fallback model against the locked profile; run documentation governance and `git diff --check`. If a code revision is justified, add targeted lifecycle/trace proof and build it under a revised packet. |
| Expected Markers | Each of READY, HOLD, HLDA, DMA request/grant, FDC DRQ/DMA2 and logical INTA is either source-qualified with scope or has one exact transfer; no scalar is silently inferred from a signal name, minimum CPU bound or reference-model default. |
| Asset Needs | No protected ROM, diagnostics disk, guest media, third-party source, binary, or trace. Read public documentation only; retain links and neutral conclusions, not copies. |
| Reporting Requirements | Distinguish source facts, inference, current implementation, and non-admission; name the future S or TODO receiver for every missing fact and report whether S3 can be admitted. |
| Stop Conditions | Stop implementation and record the transfer if IBM/Intel do not connect a selected board condition to an executable availability rule, or if a fallback model is configuration-incompatible. Pause for a packet revision if sources instead prove an implementation scope beyond the listed files. |
| Exit Criteria | P1 contains an indexed source-to-signal reconciliation and an unambiguous decision: one bounded contract for S3, or no contract with a precise source/probe transfer. It makes no model-L3 claim. |
| Original Owner Request | Continue comprehensive L3 execution in queue order before Windows 3.1; use 86Box/MAME/PCjs only as bounded fallback references where the authority does not determine timing. |
| Similar-Issue Sweep | Sweep READY/HOLD/HLDA/DMA/INTA references across selected profile, CPU transaction, DMA, PIC, FDC and evidence sources; classify each hit as primary fact, secondary check, current-route fact, or transfer. |

## Current Technical Baseline

- **Current developer artifact:** T362 S2 `vm-0-5-0362` /
  `build/output/nxvm_0_5_0362.exe`; its SHA-256 and source commit are
  recorded in the T362 S2 acceptance evidence.
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
| T369 S1 | Accepted at `87460471`: completes the Model 339 primary/secondary source disposition and production-route inventory. It admits no timing scalar: IBM/Intel define topology and CPU signal boundaries, while the examined 86Box/MAME/PCjs models cannot establish a comparable board value. S2 is the source-and-contract receiver; no 5170 L3 claim. [Ledger](../etc/evidence/t369-s1-pcat-bus-fact-route-ledger.md). |
| T368 | Closed at `4da84be8`: completes the source-labelled 80286 successful-retirement CPU ledger, with exact values or declared source-undefined transfers. The next Queue receiver is bus-timed PC/AT operation; CPU waits, bus/device timing and IBM 5170 L3 remain open. [Closure audit](../etc/evidence/t368-s7-80286-retirement-closure-audit.md). |
| T367 | Closed at `f60d87ea`: concrete machine selection and CPU/timing contract binding are VM-owned; the default-PC/AT option path now selects a VM contract before the sole core materialization boundary, while Model 339 remains descriptor-selected. Focused profile/session regressions pass; an unrelated platform-request compile failure blocks a full-gate claim. No CPU-timing, bus, device or L3 receiver is closed. [Binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md). |
| T366 | Closed at `743edc18`: locks the Model 339 baseline, planar-parity NMI and selected topology, and accepts bounded 80286 source-retirement rows. Complete CPU retirement, bus availability, device service timing and profile-L3 closure transfer explicitly; **5170 model-L3 is not ready**. [Closure audit](../etc/evidence/t366-s32-closure-transfer-audit.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |
| T364 | Closed at `7d574ae3`: all selected PC/AT components, ports/routes and lifecycle/timing owners are inventoried; optional and physical gaps retain exact Queue/TODO receivers. [Closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md). No model-L3 claim. |
| T363 | Closed at `312ef2f9`: all 256 primary and 256 secondary dispatch slots have one source-backed successful-retirement owner or an exact explicit receiver; inventory and 246/246 current-gate passed. [Closure audit](../etc/evidence/t363-s7-complete-instruction-timing-closure-audit.md). No physical/cycle-exact L3 claim. |
| T362 | Closed at `316c2334`: S1--S2 normalize every eligible 80186 immediate-IMUL source row in one private direct/constrained model table; S3 resolves stale full-gate verifier assumptions and records 246/246 current-gate tests. [Closure audit](../etc/evidence/t362-s3-legacy-timing-normalization-closure-audit.md). Model-L3 only; no physical/cycle-exact claim. |
| T361 | Closed at `a5442d96`: S1--S3 established the source-labelled 8086/80186 allocation and synchronous-fault publication boundary; S4 retained Intel 210498-005 Appendix-B `90 NOP = 3` after a page/context and consumer audit. Immediate-IMUL normalization transfers to the first Queue candidate. [S4 evidence](../etc/evidence/t361-s4-80286-nop-source-consumer-audit.md). No physical or cycle-exact claim. |

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
