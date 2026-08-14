# Project Status

## Current Work

**Active.** M5 T366 S2 locks the exact IBM 5170 baseline and project-owned
probe-corpus boundary; it allocates no availability or device timing.

## M5 T366 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner directed continued work toward comprehensive L3 precision and, on 2026-08-14, approved the 5170-first Queue order, exact baseline-machine method, and later full current-product device closure. Git pushes are permitted. |
| Objective | Select and record one exact field-configured IBM PC/AT 5170 physical baseline that can govern later NMI and bus work: Model 339 Type 3 system board, 8 MHz 80286, 512 KB system-board RAM, Rev.3 1985-11-15 ROM slot, 101-key keyboard, IBM CGA, and one explicitly identified TEAC FD-235HF-A529 3.5-inch 1.44 MB drive on the IBM diskette controller/DMA channel-2 route. The original Model 339 MFM fixed disk and serial/parallel hardware are deliberately unselected; the 1.44 MB drive is a documented field upgrade, not an IBM factory configuration. Define only the project-owned probe-corpus boundary needed to measure this configuration later. |
| Non-goals | No vendor ROM, DOS or diagnostic media import; no ROM hash catalogue; no runtime profile, wait state, NMI producer, DMA arbitration, FDC latency, serial/parallel, HDD, or display implementation; no physical/cycle-exact claim. |
| Reference Baseline | `0cb7158f`; T366 S1 inventory at `098a146a`; IBM PC/AT 5170 Models 319/339 technical material and March 1986 Technical Reference; TEAC FD-235HF-A529 specification; T365 source non-admission; [Bus-timed PC/AT operation](../proposals/m5-bus-timed-pcat-operation.md); source policy; Queue and TODO. |
| Candidate Proposal | [Bus-timed PC/AT operation](../proposals/m5-bus-timed-pcat-operation.md). |
| Files And ABI Surface | `docs/states/CURRENT.md`, T366 history/proposal and evidence/index records only. No public ABI, runtime behavior, artifact revision, firmware, guest media, or third-party source. |
| Applicable Rules | Task Reading Set; Execution packet, continuation, evidence, closure and push rules; Documentation topology; Architecture ownership/lifecycle rules; source and research policy. |
| Verification | Reconcile every selected/non-selected component with the Model 339 Type 3 board/ROM and TEAC-drive contracts; verify all source links, Queue/TODO receivers, documentation governance, and diff check. |
| Expected Markers | One T366 S2 profile-lock evidence record names the exact Model 339 field configuration, primary-source basis, local-only ROM constraint, project-owned probe categories, NMI candidate boundary, and every excluded device receiver. |
| Asset Needs | Public manual/announcement metadata only. Firmware, DOS, diagnostics, disk images, ROM bytes, hashes, local paths and third-party source remain outside the repository. |
| Reporting Requirements | Report the exact Model 339 field configuration, the correction from the non-physical 360 KB FDC assumption to the selected 1.44 MB drive, source/probe boundary, NMI handoff, MFM/ST-506 TODO receiver, excluded-device receivers, no-runtime-change result, verification and pushed commit. |
| Stop Conditions | Stop without a selection if Model 339/Type 3/ROM/drive/CGA provenance conflicts, if any selected source lacks a primary contract, or if the record would imply a vendor asset or timing value. |
| Exit Criteria | A reproducible, source-labelled 5170 baseline and project-owned probe-corpus design exist; it permits the next T366 NMI-source S but contains no timing allocation or unsupported device claim. |
| Original Owner Request | Follow the current Queue to comprehensively realize L3 precision before entering Windows 3.1 work. |
| Similar-Issue Sweep | Recheck Queue, TODO, Roadmap, Goal, T366 S1, T365 receiver, current capability baseline, and all current 5170 proposal references for board/model/drive contradictions, stale 360 KB claims, hidden vendor-asset assumptions, and unowned selected devices. |

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
| T366 S1 | Accepted at `098a146a`: the deterministic default composition has synchronous transaction ownership but no selected board/revision, wait-state contract, or firmware/DOS timing corpus. [Inventory](../etc/evidence/t366-s1-pcat-bus-profile-route-inventory.md). No timing allocation. |
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
