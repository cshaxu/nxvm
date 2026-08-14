# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | T366 S17, numbered continuation implementation subtask; P1 is one complete pushed implementation/evidence delivery. |
| Admission And Approval | The owner approved continued work toward comprehensive L3 closure on 2026-08-14. T366's CPU-ledger prerequisite admits the next determinate, successful 80286 form family: unprefixed `9E SAHF` and `9F LAHF`. No exception is requested. |
| Objective | Publish the fixed two-clock 80286 source value for successful unprefixed LAHF/SAHF through the existing retirement owner, with focused proof and durable evidence. |
| Non-goals | No FLAGS semantic repair, prefix/fault/delivery timing, memory/bus wait, physical time, device timing, other CPU profile, x87, or ABI work. Faulting paths receive no success-path cost. |
| Reference Baseline | `c2b64f3d` (accepted T366 S16); [T366 proposal](../proposals/m5-bus-timed-pcat-operation.md), [S15 evidence](../etc/evidence/t366-s15-80286-flags-control-timing.md), [S16 evidence](../etc/evidence/t366-s16-80286-xlat-timing.md), and the existing LAHF/SAHF semantic smoke. |
| Candidate Proposal | [Bus-timed PC/AT operation](../proposals/m5-bus-timed-pcat-operation.md), restricted to its source-backed 80286 successful-retirement prerequisite. |
| Files And ABI Surface | Private `machine.c` 80286 classifier, focused timing-ledger smoke, T366 evidence/index/history/status. No public header, ABI, device, composition, or execution-semantics surface changes. |
| Applicable Rules | [Execution](../rules/EXECUTION.md): complete pushed P and similar-issue sweep. [Architecture](../rules/ARCHITECTURE.md): retain one retirement owner. [Coding](../rules/CODING.md): use owner-local classifier/test paths. [Documentation](../rules/DOCUMENT.md): truthful current status. [Source policy](../etc/operations/policy/source-policy.md): read-only documentation, no imported code/assets. |
| Verification | Extend the focused 80286 timing-ledger smoke to prove `9E` and `9F` each retire at two ticks while retaining expected FLAGS/AH semantics; run its target and executable, `run-current-smokes`, documentation governance and `git diff --check`; review all changed files before P1. |
| Expected Markers | `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`; source evidence states LAHF and SAHF are two clocks in both 80286 modes. |
| Asset Needs | No guest media, ROM, binary, local path, external source import, or emulator output. AMD's 80286 instruction summary is read-only primary documentation. |
| Reporting Requirements | Report source determination, implementation/test progress, P1 push/evidence, and acceptance or one consolidated corrective brief. |
| Stop Conditions | Stop if source value/mode scope is ambiguous, a higher timing owner intercepts either opcode, success cannot be separated from a fault, semantics need repair, or a required gate cannot run. |
| Exit Criteria | A pushed P proves only successful unprefixed 80286 LAHF/SAHF publish two ticks through the existing owner; source/sweep/transfers are recorded; all checks pass; no public/device behavior changes. |
| Original Owner Request | Continue planned work until comprehensive L3 timing fidelity closes before Windows 3.1; use 86Box/MAME/PCjs only when primary sources fail to determine timing. |
| Similar-Issue Sweep | Audit `9E/9F` dispatch/handlers, primary/control-stack and all profile classifiers, 80286 fallback/default routing, existing semantic/fault coverage and focused timing proof. Retain prefixes, delivery/faults, other profiles, bus/device service and physical time at their named receivers. |

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
| T366 S16 | Accepted at `ac5d6ec6`: successful unprefixed `XLAT` publishes its fixed five-clock 80286 table cost through the existing retirement owner. Prefix, fault/delivery, bus and physical-time timing remain transferred. [Timing evidence](../etc/evidence/t366-s16-80286-xlat-timing.md). |
| T365 | Closed at `febc9352`: IBM PC/AT parity/I/O-check NMI sources cannot be selected without a profile input, status/latch/clear and lifecycle contract; CPU and CMOS remain delivery/mask only. [Closure audit](../etc/evidence/t365-s2-pcat-nmi-nonadmission-closure-audit.md). Blocks physical/cycle-exact L3 closure; no synthetic source. |
| T364 | Closed at `7d574ae3`: all selected PC/AT components, ports/routes and lifecycle/timing owners are inventoried; optional and physical gaps retain exact Queue/TODO receivers. [Closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md). No model-L3 claim. |
| T363 | Closed at `312ef2f9`: all 256 primary and 256 secondary dispatch slots have one source-backed successful-retirement owner or an exact explicit receiver; inventory and 246/246 current-gate passed. [Closure audit](../etc/evidence/t363-s7-complete-instruction-timing-closure-audit.md). No physical/cycle-exact L3 claim. |
| T362 | Closed at `316c2334`: S1--S2 normalize every eligible 80186 immediate-IMUL source row in one private direct/constrained model table; S3 resolves stale full-gate verifier assumptions and records 246/246 current-gate tests. [Closure audit](../etc/evidence/t362-s3-legacy-timing-normalization-closure-audit.md). Model-L3 only; no physical/cycle-exact claim. |
| T361 | Closed at `a5442d96`: S1--S3 established the source-labelled 8086/80186 allocation and synchronous-fault publication boundary; S4 retained Intel 210498-005 Appendix-B `90 NOP = 3` after a page/context and consumer audit. Immediate-IMUL normalization transfers to the first Queue candidate. [S4 evidence](../etc/evidence/t361-s4-80286-nop-source-consumer-audit.md). No physical or cycle-exact claim. |
| T360 | Closed four-profile timing source reconciliation: primary-authority inventory, owner-selected 80286 NOP table precedence, exact legacy range transfer, contextual classifier audit, and final source/receiver closure. 245/245 current-gate tests passed; no L3 or cycle-exact claim. [Closure audit](../etc/evidence/t360-s5-four-profile-timing-source-closure-audit.md). |
| T359 | Closed its bounded four-profile instruction-retirement corpus: S1 inventory plus S2--S6 source classifiers use one successful-retirement publisher; S7 verifies all unallocated rows transfer to T360, bus/service, cycle-exact, or x87/VME receivers. 245/245 current-gate tests passed. No L3 or cycle-exact claim is made. [Closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |

## Recent Governance

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
