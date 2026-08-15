# Project Status

## Current Work

## M5 T369 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; T369 S3 accepted at `037cf13b`. |
| Admission And Approval | The owner-approved instruction to continue the ordered L3 program authorizes closure of the open T369 package. The audit may close only the source-labelled bus-stage scope or transfer an unresolved boundary; it may not claim 5170 L3. |
| Objective | Audit T369 S1--S3 against its candidate scope: selected Model-339 input, CPU memory/port availability, logical DMA exclusion, selected FDC/PIC visibility, trace/reset lifecycle and exact downstream receivers. Close T369 only if every scoped boundary is implemented or explicitly transferred through the ordered Queue/TODO path. |
| Non-goals | No new timing scalar, physical duration/waveform, CPU microstep/resume, device-service model, logical-to-physical INTA equivalence, new device/adapter, topology change, MFM/ATA change, or 5170-L3 claim. |
| Reference Baseline | T369 S1/S2/S3 evidence and task record; T366 Model-339/NMI closure; T368 CPU ledger closure; T354 transaction/competition evidence; current Queue, TODO and the Model-339 descriptor/CPU-DMA-PIC-FDC routes. |
| Candidate Proposal | [Bus-timed PC/AT operation](../proposals/m5-bus-timed-pcat-operation.md). |
| Files And ABI Surface | Audit evidence, T369 history, Current status and evidence index only. If an unclassified production defect is found, stop and revise the packet before altering code or ABI. |
| Applicable Rules | Documentation authority boundaries; execution closure, artifact, source-policy and similar-issue requirements; architecture one-owner, validation-before-publication and lifecycle/reset/trace invariants. Existing CPU source-retirement ticks remain the sole CPU-time publisher. |
| Verification | Reinspect all S1--S3 evidence and actual production routes; rebuild `vm-0-5-0369`, verify the current artifact target, rerun the retained transaction/competition/lifecycle smokes, sweep transaction/DMA/PIC/FDC/reset/trace routes, run documentation governance and diff check. |
| Expected Markers | Retain `M5:T354:S2:TRANSACTION:OK`, `M5:T354:S3:COMPETITION:OK`, `M5:T369:S3:PCAT-HOLD:OK`, `M5:T354:S4:TRANSACTION-LIFECYCLE:OK` and `M5:T197:S1:CURRENT-ARTIFACT-TARGET:vm-0-5-0369:OK`. |
| Asset Needs | No ROM, guest media, third-party code, binary or raw trace. Existing public manuals and project-authored evidence only. |
| Reporting Requirements | Map each required bus-stage boundary to implementation or a precise receiver; record artifact identity and source commit; distinguish logical ordering from physical timing; state explicitly whether T369 and 5170 L3 close. |
| Stop Conditions | Stop and revise if the audit requires a new scheduler, CPU preemption, timing scalar, physical waveform assertion, device service behavior, public product/VM ABI, or an unclassified source/route repair. |
| Exit Criteria | P1 produces an evidence-backed T369 closure/transfer audit, closes T369 only when its bounded bus-stage scope has one owner or exact receiver for every route, and leaves T370/phase/5170-L3 work explicit. It must not present a logical handoff or deterministic ordering as physical/cycle-exact timing. |
| Original Owner Request | Continue in Queue order toward complete L3 before Windows 3.1; use named reference models only where authoritative manuals leave a range or no range, without treating them as IBM authority. |
| Similar-Issue Sweep | Sweep Model-339 descriptor, CPU memory/port transaction helpers, transaction begin/commit/cancel/hold APIs, DMA request/advance, PIC refresh/acknowledgement, FDC request/advance/refresh, reset/finalize, trace consumers and stopped/paused external operations; classify every hit as retained owner, transfer or packet-revision blocker. |

## Current Technical Baseline

- **Current developer artifact:** T369 S3 `vm-0-5-0369` /
  `build/output/nxvm_0_5_0369.exe`; its SHA-256 and source commit are
  recorded in the T369 S3 acceptance evidence.
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
| T369 S3 | P1 `12ac65ac` adds and proves one source-labelled logical 80286 DMA HOLD lifecycle at the shared arbitration boundary, with copied trace and reset release. It produces `nxvm_0_5_0369.exe`; board waits, physical waveforms, device service, remaining CPU work and 5170 L3 remain transferred. [S3 evidence](../etc/evidence/t369-s3-pcat-logical-hold-lifecycle.md). |
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
