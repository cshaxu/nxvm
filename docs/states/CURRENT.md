# Project Status

## Current Work

**Active implementation packet:** M5 T366 S3, selected Model 339
system-board-memory-parity NMI lifecycle.

## M5 T366 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository owner. Approval: the owner's continuing instruction to implement T361 and its successors through the M5 L3 closure, and the 2026-08-14 approved Model 339 Type 3 baseline; the latter explicitly directs a no-fixed-disk 5170, distinguishes ATA/HDC from future MFM/ST-506, and restricts reference emulators to cross-checking. Approved scope: only the selected system-board RAM-parity NMI lifecycle needed before bus availability. No exception is approved. |
| Objective | Implement one core-owned, opt-in system-board RAM-parity controller with a documented port `61h` status/control contract, CMOS `70h` NMI-mask interaction, reset/finalize behavior, deterministic pending-NMI visibility, and real/protected/ordinary-VM86 delivery coverage. The controller is not a new generic machine baseline: only a later selected-profile composition may enable it. |
| Non-goals | Do not change the current default PC/AT CPU, RAM, ATA/HDC, FDC, display, ROM, or boot topology; claim it is Model 339; implement I/O-channel check, expansion-memory parity, MFM/ST-506, speaker/timer port-B behavior beyond preserving its owned bits, bus waits, DMA/HOLD, or physical/cycle timing; import firmware/media/reference code; or use RTC, CMOS, PIC, or CPU delivery state as an NMI producer. |
| Reference Baseline | T366 S2 accepted at `0e04b405`, with the IBM PC/AT Technical Reference March 1986 as primary behavioral authority and its selected Model 339 Type 3 scope in [profile lock](../etc/evidence/t366-s2-5170-profile-lock.md). T365's [non-admission decision](../etc/evidence/t365-s1-pcat-nmi-source-decision.md) remains the source/receiver audit. The current implementation has CPU `flagNMI` delivery and CMOS masking but no `61h` or parity owner. |
| Candidate Proposal | [Bus-Timed PC/AT Operation](../proposals/m5-bus-timed-pcat-operation.md), NMI-source stage only. |
| Files And ABI Surface | Expected: `src/core/machine/{machine.c,machine.h,machine_interface.h}` plus a cohesive parity owner if justified; default profile/session composition only for opt-in registration; focused machine/product tests and CMake test registration; `docs/etc/evidence/`, `docs/etc/README.md`, `docs/history/M5-T366-bus-timed-pcat-operation.md`, and this packet. A public operation may expose only a bounded configuration and copied observation contract, never a raw machine/CPU/RAM/device pointer; no test-only public injection ABI. |
| Applicable Rules | Execution: packet, one-S/P lifecycle, self-review, similar-issue sweep and durable evidence. Architecture: one mutable-state owner and composition-only integration; core remains VM-neutral; no raw mutable cross-module ABI. Coding: C11, one cohesive owner, no test mirror or test-only public contract. Documentation: evidence indexed and active status only here. Source policy: IBM material remains documentation/research only, with no imported listing, ROM, media, or vendor binary. All apply; no rule is waived. |
| Verification | Build and run a focused parity/NMI smoke covering clean state, board-parity assertion/latch, `61h` status, `61h` disable/enable clear sequence, `70h` mask deferral/release, reset/finalize, and real/protected/ordinary-VM86 NMI delivery; run every existing PC/AT topology/composition test affected by the added port; run the current required test gate and documentation-governance gate; run `git diff --check`. Record exact commands and markers in indexed evidence. |
| Expected Markers | Focused proof emits `M5:T366:S3:PLANAR-PARITY-NMI:OK`; existing PC/AT checks retain their current markers; governance completes successfully. |
| Asset Needs | None. Tests use repository-authored code only. No ROM, DOS, diagnostics, guest media, emulator trace, or third-party source is required. |
| Reporting Requirements | Report the initial contract confirmation, a progress update after the owner/latch/control path is integrated, and final pushed P with evidence, verification, changed ownership boundary, and explicit transfers. |
| Stop Conditions | Stop and return to the owner if IBM authority cannot determine the selected Type-3 port/latch semantics; an actual 5170 profile change becomes necessary; port-B ownership cannot preserve unrelated speaker/timer bits; a public synthetic fault-injection ABI would be needed; any selected route requires I/O-channel check, unselected hardware, timing allocation, or external/protected asset. |
| Exit Criteria | One configuration-gated system-board parity owner has an assertion source, latch/status, documented `61h` clear path, `70h` mask relationship, reset/finalize behavior, and delivery proof in each named CPU mode. No default profile misidentification or unselected I/O-check/storage/timing behavior is introduced; tests and evidence prove it. |
| Original Owner Request | The owner confirmed the machine direction: use the late IBM PC/AT 5170 Model 339 / Type 3 as the 80286 L3 baseline (8 MHz, Rev.3 BIOS dated 1985-11-15, 512 KB planar, 101-key keyboard), initially without a hard disk; use a named 3.5-inch 1.44 MB drive only as a documented field upgrade; do not add 1 MB expansion; retain future MFM/ST-506 disk/controller work in TODO; do not call the current ATA/HDC model IBM 30 MB MFM; and use 86Box `ibmat`/MAME `ibm5170a` solely for behavioral cross-checking, never as exact-machine authority. |
| Similar-Issue Sweep | Inspect all port `61h`, parity, channel-check, `flagNMI`, NMI-mask, reset/finalize, configuration rollback, profile leaf, and CPU-mode delivery paths. Repair the shared ownership/lifecycle mechanism or transfer each excluded source; do not add a parallel device or direct CPU-state write. |

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
| T366 S2 | Accepted at `0e04b405`: locks a field-configured IBM 5170 Model 339 Type 3 baseline--8 MHz, 512 KB, Rev.3 ROM slot, 101-key keyboard, CGA and TEAC 1.44 MB drive--with no fixed disk or serial/parallel adapter. ATA/HDC is explicitly distinct from the deferred MFM/ST-506 path. [Profile lock](../etc/evidence/t366-s2-5170-profile-lock.md). No timing allocation. |
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
