# Project Status

## Current Work

**Active.** M5 T362 S1 is the source-domain and normalization ledger for the
remaining legacy dynamic-arithmetic timing routes.

## M5 T362 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner explicitly directed execution of the current Queue through the complete instruction-timing corpus on 2026-08-14. This admits Queue's first candidate after Td S90 withdrew earlier unstarted preparation. Git pushes are permitted. |
| Objective | Establish a form-by-form eligibility ledger for Intel-constrained legacy timing normalization: Intel bounds, exact profile/form, external scalar/revision, EA/prefix/odd-word accounting basis, raw and normalized value, and direct/clamped/reference-exhausted disposition. |
| Non-goals | No runtime allocation, timing-value change, fallback removal, CPU/machine boundary change, bus/prefetch/device/service timing, physical measurement, cross-profile borrowing, third-party source import, 80286/80386 allocation, or cycle-exact claim. |
| Reference Baseline | `1484a7fa` / `vm-0-5-0359`; [Intel-constrained normalization proposal](../proposals/m5-intel-constrained-legacy-timing-normalization.md), T361 S1--S3 evidence, and the current 8086/80186 timing ledgers. |
| Candidate Proposal | [Intel-constrained legacy timing-model normalization](../proposals/m5-intel-constrained-legacy-timing-normalization.md). |
| Files And ABI Surface | Research/evidence, T362 history, `docs/states/CURRENT.md`, and source/test inventory only. No runtime source, public ABI, build target, dependency, firmware, media, or artifact change. |
| Applicable Rules | Task Reading Set; Execution packet/P/closure and similar-issue rules; Documentation authority boundaries; source/research policy; Architecture source-admission and single-owner invariants. Coding rules are not triggered because S1 makes no code change. |
| Verification | Enumerate all transferred T361 forms, verify primary Intel row/range and MAME/86Box/PCjs applicability, distinguish scalar accounting from machine/bus components, record lower/upper clamp eligibility, sweep current one-tick fallbacks, and run documentation governance plus diff check. |
| Expected Markers | No new runtime marker; the durable `T362 S1` evidence ledger identifies every scoped form and its disposition. |
| Asset Needs | Read-only T361 evidence, Intel manuals, and referenced implementation revisions; no retained download, guest media, protected binary, hardware measurement, or source import. |
| Reporting Requirements | Report the eligibility matrix before any allocation proposal. Record every rejected reference and exact reason, and identify whether S2 can implement central allocation without a scope revision. |
| Stop Conditions | Stop a row as reference-exhausted if exact profile/form, Intel range, source revision, or accounting basis is unproven. Stop the S if an eligible row requires a machine/bus/device owner or a broader CPU refactor. Do not infer a scalar, midpoint, or cross-generation substitute. |
| Exit Criteria | Durable evidence covers every scoped form and fallback, names all source/revision/accounting comparisons, distinguishes direct/clamped/reference-exhausted outcomes, and gives a bounded S2 implementation surface or a precise transfer. |
| Original Owner Request | Execute the current Queue through complete instruction timing; use Intel manuals first and 86Box/MAME/PCjs only where their applicable model evidence fills a non-deterministic primary record. |
| Similar-Issue Sweep | Search all tracked machine source, timing smokes, CMake, T361 evidence, and current documentation for legacy Group-3/immediate-IMUL dynamic forms and `CORE_MACHINE_SOURCE_UNALLOCATED_TICKS`; classify each matching fallback and its owner. |

## Current Technical Baseline

- **Current developer artifact:** T359 S6 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `65823D830715FC8925DBD810C2BCDEC3AEEB654D3B6B96BB022870ABF9B6783D`.
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
| T361 | Closed at `a5442d96`: S1--S3 established the source-labelled 8086/80186 allocation and synchronous-fault publication boundary; S4 retained Intel 210498-005 Appendix-B `90 NOP = 3` after a page/context and consumer audit. Immediate-IMUL normalization transfers to the first Queue candidate. [S4 evidence](../etc/evidence/t361-s4-80286-nop-source-consumer-audit.md). No physical or cycle-exact claim. |
| T360 | Closed four-profile timing source reconciliation: primary-authority inventory, owner-selected 80286 NOP table precedence, exact legacy range transfer, contextual classifier audit, and final source/receiver closure. 245/245 current-gate tests passed; no L3 or cycle-exact claim. [Closure audit](../etc/evidence/t360-s5-four-profile-timing-source-closure-audit.md). |
| T359 | Closed its bounded four-profile instruction-retirement corpus: S1 inventory plus S2--S6 source classifiers use one successful-retirement publisher; S7 verifies all unallocated rows transfer to T360, bus/service, cycle-exact, or x87/VME receivers. 245/245 current-gate tests passed. No L3 or cycle-exact claim is made. [Closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |

## Recent Governance

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

- **M5 Td S86:** replaced the completed instruction-timing TODO with ordered
  candidates for the remaining full instruction corpus and the independent
  80286 Appendix-B source reconciliation; retained bus-timed and cycle-exact
  work in their dependency order. Documentation governance and diff check
  passed; Td work has no runtime change.

- **M5 Td S85:** corrected the Windows readiness candidate so FDC/ATA
  pending/readiness service is an accepted regression baseline, rather than a
  stale missing-feature blocker. Physical device timing and Windows
  compatibility remain unclaimed. Documentation governance and diff check
  passed; Td work has no runtime change.

- **M5 Td S84:** retired the stale FDC/ATA command-service TODO after the
  accepted T347 pending-command/readiness-service closure and T354 consumer
  audit reconfirmed its claim was no longer true. Physical wait/cycle fidelity
  remains in the existing bus-timed and cycle-exact debt entries. Documentation
  governance and diff check passed; Td work has no runtime change.
