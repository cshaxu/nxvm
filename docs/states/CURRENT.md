# Project Status

## Current Work

**Active implementation packet:** M5 T366 S7, Model 339 firmware and FDC
topology boundary.

## M5 T366 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository owner. Approval: persistent M5 L3 direction, the approved 2026-08-14 late IBM PC/AT 5170 Model 339 / Type 3 baseline, and accepted T366 S6 transfer. Scope: bind the selected abstract Rev.3 BIOS slot and TEAC FD-235HF-A529 1.44 MB field-upgrade FDC topology to the Model 339 profile, or make an evidence-backed exact transfer. No exception is approved. |
| Objective | Reconcile the selected profile's firmware-slot identity, no-vendor-ROM boundary, diskette drive geometry/type, FDC controller/ports, DMA channel 2, IRQ6, reset and lifecycle with one Model 339 configuration. Make the field upgrade explicit without treating it as factory hardware. |
| Non-goals | Do not import, hash, catalogue or map IBM ROM bytes; do not claim BIOS behavior, firmware timing, exhaustive 765-family command timing, MFM/ST-506, ATA/HDC, serial/parallel cards, I/O-channel check, CGA raster timing, bus waits, arbitration, refresh or physical/cycle timing. Do not add a 1 MB memory expansion. |
| Reference Baseline | T366 S2 fixes a late 8 MHz Model 339 Type 3 with 512 KB planar RAM, Rev.3 BIOS slot, IBM CGA and a TEAC 1.44 MB field upgrade. T366 S5 selects no ATA/HDC and S6 selects CGA-only topology. Existing generic FDC composition must be inspected rather than presumed to represent that field upgrade. |
| Candidate Proposal | [Bus-Timed PC/AT Operation](../proposals/m5-bus-timed-pcat-operation.md), which requires selected diskette/FDC DMA-channel-2 topology before bus availability. |
| Files And ABI Surface | Expected: profile descriptor/composition/FDC configuration only if the selected topology can be expressed honestly; focused proof/CMake registration; indexed evidence, T366 history and current packet. Firmware remains an abstract profile slot; no vendor asset or machine-local manifest enters the repository. |
| Applicable Rules | Execution packet/evidence/acceptance and similar-issue sweep; source policy governs firmware and external reference research; architecture gives composition/profile one topology selector and FDC one controller owner; coding requires C11/cohesive lifecycle configuration; documentation requires indexed evidence/status-only packet. All apply; no waiver. |
| Verification | Build a source-to-profile-to-composition ledger. Focused proof must distinguish Model 339 field-upgrade diskette/FDC selection from default composition and prove FDC port, IRQ6, DMA2 and reset/lifecycle ownership; run affected FDC/profile/session tests, current smoke gate, documentation governance and `git diff --check`. If source evidence is insufficient, prove the exact non-admission path and transfer it without a guessed topology. |
| Expected Markers | A new focused S7 marker must identify the accepted profile/FDC boundary; existing FDC, session and current-gate markers remain successful. |
| Asset Needs | Public manuals and technical references may be read under source policy; no firmware, guest media, ROM hash catalogue, downloads or machine-local paths may be committed. |
| Reporting Requirements | Report authoritative source basis, field-upgrade versus factory distinction, firmware abstraction, selected FDC route/lifecycle, retained default proof, final pushed evidence and every transfer. |
| Stop Conditions | Stop if the exact FDC/drive or firmware-slot contract cannot be supported without vendor assets, if generic FDC behavior would be misrepresented as IBM-specific timing, or if the selected upgrade conflicts with the Model 339 baseline. |
| Exit Criteria | The selected Model 339 either owns an explicit non-vendor firmware slot and field-upgrade FDC topology with route/lifecycle proof, or has one exact evidence-backed transfer; no ATA/MFM or factory-upgrade claim leaks into the profile, and all timing breadth remains transferred. |
| Original Owner Request | Use late Model 339 / Type 3 as the 286 L3 baseline: 8 MHz, Rev.3 BIOS 1985-11-15, 512 KB board memory, 101-key keyboard, IBM CGA, selected 1.44 MB post-sale upgrade, no initial fixed disk; leave MFM/ST-506 in TODO and do not describe current ATA/HDC as IBM MFM. |
| Similar-Issue Sweep | Inspect all profile firmware declarations, diskette-drive configuration, FDC port/IRQ/DMA/reset and session lifecycle paths, default-versus-Model selection tests, and source/evidence wording. Repair one shared profile/composition owner or transfer each unsupported model-specific claim to the exact later receiver. |

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
| T366 S6 | Accepted at `dd464d74`: Model 339 selects one CGA-only VADP topology with VADP-owned B8000h VRAM, while generic PC/AT retains its EGA ports and aperture. [Topology evidence](../etc/evidence/t366-s6-model-339-cga-topology.md). IBM ROM, exhaustive CGA timing, FDC/MFM-ST-506 and bus timing remain transfers. |
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
