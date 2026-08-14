# Project Status

## Current Work

**Active implementation packet:** M5 T366 S4, Model 339 planar-memory/parity
binding.

## M5 T366 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository owner. Approval: continuing M5 L3 implementation direction and the approved 2026-08-14 Model 339 Type 3 baseline. Scope: make the selected 512 KB system-board RAM and its S3 parity source a truthful, bounded core/profile contract before bus availability. No exception is approved. |
| Objective | Permit a 512 KB planar-memory configuration; bind an opt-in Model 339 planar-RAM parity owner to the one shared physical-memory read/write path; establish stored parity on writes and parity fault assertion on mismatched reads; preserve reset, mapping and source-lifecycle ownership. |
| Non-goals | Do not replace the generic default PC/AT profile; claim its 80386/ATA configuration is a 5170; select I/O-channel check, expansion memory, speaker/timer, MFM/ST-506, 1 MB expansion, bus waits, DMA arbitration, refresh timing, or any cycle timing. Do not add a test-only public corruption API or import protected material. |
| Reference Baseline | T366 S2 Model 339 lock at `0e04b405`; T366 S3 parity source accepted at `b7bfe7a4`; current core minimum memory is 2 MB while `memory.c` owns the CPU/DMA/firmware/display physical read/write route. |
| Candidate Proposal | [Bus-Timed PC/AT Operation](../proposals/m5-bus-timed-pcat-operation.md), profile/NMI stage before the bus stage. |
| Files And ABI Surface | Expected: shared core memory/machine configuration and parity state, an explicitly selected VM profile/composition boundary if needed, focused tests/CMake registration, current packet, evidence/index and T366 history. Public contracts may expose configuration and copied observation only; no raw RAM/CPU pointer. |
| Applicable Rules | Execution packet/evidence/acceptance; architecture one shared RAM and source owner with composition-only integration; coding C11/cohesive owner/no test mirror; documentation indexed evidence/status-only packet; source policy documentation/research only. All apply; no waiver. |
| Verification | Focused proof must create 512 KB, establish good parity through CPU and DMA-visible writes, detect a controlled owner-local stored-parity mismatch through a shared read, prove `61h`/`70h` lifecycle and reset/reconfigure behavior, and retain non-parity memory routes. Run affected profile/topology tests, current smoke gate, documentation governance and `git diff --check`. |
| Expected Markers | Focused proof emits `M5:T366:S4:PLANAR-MEMORY-PARITY:OK`; existing current-gate markers remain successful. |
| Asset Needs | None; repository-authored code and fixtures only. |
| Reporting Requirements | Report contract confirmation, shared-route implementation progress, and final pushed evidence/verification/transfers. |
| Stop Conditions | Stop for missing authority on 512 KB planar parity behavior, if a second RAM path/public corruption API is required, if low-memory mappings cannot be preserved, or if scope requires unselected I/O channel, devices or timing. |
| Exit Criteria | One 512 KB-admissible configuration and one shared memory parity binding prove write-establish/read-check/fault/latch/mask/reset behavior without changing default PC/AT identity; every excluded source retains its receiver. |
| Original Owner Request | Use the late IBM PC/AT 5170 Model 339 / Type 3 as the 80286 L3 baseline with 512 KB planar memory and no fixed disk, while retaining separate ATA/HDC and deferred MFM/ST-506 paths. |
| Similar-Issue Sweep | Inspect core memory allocation, every physical read/write caller, mappings/providers, reset/reconfigure, profile memory declarations, parity/NMI/port-61 control and all lower-memory checks. Repair the shared mechanism or transfer excluded variants. |

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
| T366 S3 | Accepted at `b7bfe7a4`: core owns an opt-in Model 339 planar-RAM-parity source at `61h`; its latch/status, bit-2 clear/re-enable, `70h` NMI-mask interaction and reset are proven without altering the generic PC/AT profile. [Lifecycle evidence](../etc/evidence/t366-s3-planar-parity-nmi.md). I/O-channel check, physical parity-memory binding and timing remain transfers. |
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
