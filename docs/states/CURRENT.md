# Project Status

## Current Work

**Active.** M5 T361 remains open between accepted S units. S3 accepted a
range-checked model allocation and synchronous-fault execution-round boundary;
the separately bounded 80286 `NOP` review is the next admissible S.

## Accepted M5 T361 S3 Packet (historical)

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved this S on 2026-08-14 and clarified the source order in the current request: exclude unavailable real-hardware measurement; use Intel/manual values first, then 86Box for early-PC timing, MAME i86/i186 for 80186 timing, and PCjs as an 8086 model; inspect Bochs, PC110-EMU, and QEMU for their narrower relevance. Use an existing model for an Intel range only after range comparison, and use a documented same-profile model where no primary value exists. |
| Objective | Audit 86Box, MAME i86/i186, PCjs, QEMU, Bochs, PC110-EMU, and any publicly found same-profile candidate for every T361 8086/80186 dynamic-arithmetic form; apply the exact-Intel, Intel-range-constrained external model, no-Intel external model, or reference-exhausted ladder, and implement only values justified by that policy. Reconcile the shared execution-round boundary so an original instruction whose synchronous exception is successfully delivered does not retire or publish instruction ticks, while its delivery state remains committed. |
| Non-goals | No claim that emulator values are physical measurements; no copied/imported third-party code or dependency; no hardware measurement, host benchmark, self-referential NXVM fallback, bus/prefetch/device timing, generic arithmetic refactor, 80286/80386 allocation, or physical cycle-exact claim. Do not alter exception frame layout, vector selection, IDT/IVT delivery, external PIC IRQ/NMI, or post-instruction debug-trap architecture; only make the existing execution-round outcome visible to the sole publisher. The separately bounded 80286 `NOP` source/consumer review remains the next T361 S, not an S3 implementation item. |
| Reference Baseline | `3ef7f1d7` / `vm-0-5-0359`; S1/S2 evidence and [legacy dynamic arithmetic proposal](../proposals/m5-legacy-dynamic-arithmetic-timing-authority.md). |
| Candidate Proposal | [Legacy dynamic arithmetic timing authority](../proposals/m5-legacy-dynamic-arithmetic-timing-authority.md), revised under the owner's no-hardware, model-derived allocation policy. |
| Files And ABI Surface | Research evidence, timing ledgers/tests, private `src/core/machine/` execution-round outcome and timing allocation, task history, TODO/Queue disposition, and `docs/states/CURRENT.md`. No public ABI, external source import, product dependency, or firmware/media asset. |
| Applicable Rules | Read the Task Reading Set, execution S/P/mechanism clauses, documentation and architecture rules, coding rules if code changes, and source/research policy. Preserve one successful-retirement publisher; label every non-primary value as model-derived and every no-source result as reference-exhausted. |
| Verification | Enumerate each form and source revision/path; prove exact Intel values where available; for Intel ranges enumerate the selected model domain and prove every value stays in range; prove source-profile/form match, no-fault allocation and fault nonpublication. The focused execution-round matrix must prove successfully delivered synchronous `#DE/#GP/#UD/#BR/#NM/#MF/#NP/#SS/#TS/#PF` do not retire or publish original-instruction ticks; an execution-before `#DB` follows that rule; a successful instruction followed by TF `#DB` still retires once; external IRQ/NMI still retire the successful predecessor; terminal delivery failure remains zero; and handler instructions retire only on a later run. Run focused timing smokes, documentation governance, diff check, and full current-gate for code changes. |
| Expected Markers | Existing 8086 and 80186 timing-ledger smokes remain passing; any allocation proof has an `M5:T361:S3` marker. |
| Asset Needs | Read-only local PCjs, Bochs 2.6, and PC110-EMU; publicly readable 86Box, MAME i86/i186, QEMU, and discovered same-profile reference source documentation; no binary asset, guest media, download, or imported source. |
| Reporting Requirements | Record the exact relevance or non-relevance of 86Box, MAME i86/i186, and QEMU; for every form record all candidate sources, revision/path, profile/form match, formula or constant, Intel range comparison when available, rejection reason, selected provenance label, and non-import conclusion. |
| Stop Conditions | Stop allocation for a form if no consulted implementation exposes a reproducible same-profile model, if a selected range model violates Intel bounds, if source/license/import or an exception delivery/layout/vector semantic change beyond the private execution-round outcome is needed, or if references require a behavior beyond the admitted profile/form boundary. Record `reference-exhausted` precisely; do not guess or reuse NXVM's marker as evidence. |
| Exit Criteria | Every S1 form has an exact Intel, range-constrained model-derived, no-Intel model-derived, or explicit reference-exhausted disposition; every runtime value has focused consumer/fault proof and provenance; the shared execution-round boundary is proven for every listed synchronous fault class, execution-before and post-instruction `#DB`, IRQ/NMI, terminal fault, and subsequent handler retirement; QEMU, all specified local references, and all admitted public candidates are audited; closure material states model-L3 versus non-physical boundaries without an untracked residual. |
| Original Owner Request | With no physical hardware available, use documents where available; otherwise treat 86Box, MAME, and PCjs as preferred reference sources, audit QEMU, Bochs, and PC110-EMU at their applicable boundaries, and implement the most practical source-labelled L3 model. |
| Similar-Issue Sweep | Sweep every S1 transferred 8086/80186 Group-3 and immediate-IMUL form, their profile/prefix/register-memory variants, all successful-retirement consumers, and every specified external implementation; distinguish an actual formula from a constant, benchmark calibration, or absent timing model. |

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
| T361 S3 | Accepted at `914c6afc`: MAME constants are used only where they stay within Intel Table 1-16 ranges; range-invalid or incomparable immediate-IMUL forms remain reference-exhausted. Successful synchronous fault delivery stops the faulting round with zero retirement/ticks, and handler retirement begins on a later run. [Evidence](../etc/evidence/t361-s3-synchronous-fault-retirement-audit.md). No physical or cycle-exact claim. |
| T361 S2 | Accepted the S1 primary-source audit and owner-approved Bochs/PCjs/PC110/public cross-reference: neither supplies a convergent 8086/80186 dynamic-arithmetic formula; the retained one-tick marker is not L3 timing, and the exact hardware-observation contract remains required. [S1 evidence](../etc/evidence/t361-s1-legacy-dynamic-arithmetic-authority-audit.md); [S2 evidence](../etc/evidence/t361-s2-legacy-dynamic-arithmetic-cross-reference.md). |
| T360 | Closed four-profile timing source reconciliation: primary-authority inventory, owner-selected 80286 NOP table precedence, exact legacy range transfer, contextual classifier audit, and final source/receiver closure. 245/245 current-gate tests passed; no L3 or cycle-exact claim. [Closure audit](../etc/evidence/t360-s5-four-profile-timing-source-closure-audit.md). |
| T359 | Closed its bounded four-profile instruction-retirement corpus: S1 inventory plus S2--S6 source classifiers use one successful-retirement publisher; S7 verifies all unallocated rows transfer to T360, bus/service, cycle-exact, or x87/VME receivers. 245/245 current-gate tests passed. No L3 or cycle-exact claim is made. [Closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |

## Recent Governance

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

- **M5 Td S83:** corrected the NXVM self-sibling instruction in `AGENTS.md`;
  renamed the VM-platform injected test macro to
  `VM_PLATFORM_TEST_FAILURE_STAGE` across CMake, implementation, and smoke
  sources; and aligned CMake's project identity and diagnostics with NXVM.
  Documentation self-test, default governance check, exact macro/search audit,
  and diff check passed. Td work has no intended runtime or artifact behavior
  change.
