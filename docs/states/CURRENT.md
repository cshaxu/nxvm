# Project Status

## Current Work

**Active implementation packet:** M5 T366 S5, Model 339 selectable session
composition.

## M5 T366 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository owner. Approval: persistent M5 L3 direction plus the approved 2026-08-14 Model 339 Type 3 baseline. Scope: make the locked 80286/512 KB/planar-parity/no-fixed-disk selection an explicit session choice, without changing the retained generic 80386/ATA PC/AT session. No exception is approved. |
| Objective | Add one explicit Model 339 session-profile selector and descriptor; materialize its selected 80286, 512 KB planar memory, Model 339 parity binding and no-fixed-disk topology; ensure ATA/HDC configuration and hard-disk media operations are unavailable for that selection. The exact 8 MHz timing contract remains a later bus/timing S. |
| Non-goals | Do not alter the generic default PC/AT identity or its ATA/HDC behavior; claim a repository-authored BIOS is IBM Rev.3 ROM; select MFM/ST-506, I/O-channel check, expansion memory, serial/parallel hardware, 80287, exact CGA port/video behavior, FDC service timing, bus waits, arbitration, refresh or any physical/cycle timing. |
| Reference Baseline | T366 S2 profile lock at `0e04b405`, T366 S3 source lifecycle at `b7bfe7a4`, and T366 S4 shared planar parity at `ff5fbb53`; `vm_session` currently hard-wires one default 80386/ATA descriptor and configures both FDC and HDC unconditionally. |
| Candidate Proposal | [Bus-Timed PC/AT Operation](../proposals/m5-bus-timed-pcat-operation.md), profile/NMI stage before bus availability. |
| Files And ABI Surface | Expected: session selection configuration, PC/AT descriptor/validation and device/firmware composition decisions, focused composition tests/CMake registration, current packet, indexed evidence and T366 history. A public selector may expose only a stable profile identity; raw device or RAM pointers remain private. |
| Applicable Rules | Execution packet/evidence/acceptance and similar-issue sweep; architecture composition owns profile choice and device topology while core owns parity; coding C11/cohesive validation/no parallel profile path; documentation indexed evidence/status-only packet; source policy prohibits IBM ROM/media import. All apply; no waiver. |
| Verification | Focused proof must create both default and Model 339 sessions, demonstrate the selected CPU/memory/parity contract, show absent ATA port ownership and rejected HDD input/boot operations for Model 339, and prove default ATA behavior remains. Run affected session/profile topology tests, current smoke gate, documentation governance and `git diff --check`. |
| Expected Markers | New focused proof emits `M5:T366:S5:MODEL339-COMPOSITION:OK`; existing current-gate markers remain successful. |
| Asset Needs | None; repository-authored code and fixtures only. No vendor ROM, media, image, hash or local path. |
| Reporting Requirements | Report selector/ownership confirmation, the selected and deliberately unselected surfaces, focused proof and final pushed evidence/transfers. |
| Stop Conditions | Stop for any requirement to call the generic ATA HDC an IBM MFM subsystem, to install external IBM firmware/media, to make CGA/EGA behavior ambiguous, or to add a duplicate session lifecycle rather than selecting through the existing composition owner. |
| Exit Criteria | One request selects the bounded Model 339 core/device contract while the default profile remains unchanged; Model 339 has no ATA/HDC lifecycle or hard-disk input path, and every remaining hardware/firmware/timing gap has a named receiver. |
| Original Owner Request | Use the late IBM PC/AT 5170 Model 339 / Type 3 as the 80286 L3 baseline with 512 KB planar memory, 101-key keyboard, documented TEAC 1.44 MB field upgrade and no fixed disk; never mislabel the separate ATA/HDC implementation as IBM MFM. |
| Similar-Issue Sweep | Inspect every session profile assignment/validation, profile-derived core field, device setup/reset/finalize path, firmware hook, storage operation, console adapter and test fixture. Repair the shared selection mechanism or transfer unsupported Model 339 device behavior to its exact later receiver. |

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
| T366 S4 | Accepted at `ff5fbb53`: a Model 339 selection can create 512 KB planar RAM and bind stored parity to the one shared physical read/write route; mismatches retain the S3 `61h`/`70h` NMI lifecycle, while the generic 80386/ATA PC/AT identity stays unchanged. [Memory/parity evidence](../etc/evidence/t366-s4-planar-memory-parity.md). Explicit VM composition, I/O-channel check and timing remain transfers. |
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
