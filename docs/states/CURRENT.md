# Project Status

## Current Work

**Active.** M5 T360 S4 80286/80386 contextual source-row normalization is
admitted in single-session mode.

## M5 T360 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuation of T360 after accepted S3 `cb534967`, under the approved four-profile source-reconciliation proposal and the owner's direction to complete the queue in single-session mode. |
| Objective | Audit every T359-selected 80286 Appendix-B and 80386 source-clock allocation for the contextual conditions that select its row: following bytes, effective address, odd-word transfer, privilege/mode, descriptor granularity, `m`/next-instruction component count, and legal prefix form; either normalize a demonstrable source mismatch at the one retirement publisher or record an exact transfer. |
| Non-goals | No new instruction semantics, source import, timing approximation, profile backfill, host benchmark or emulator authority, bus/device/delivery/cycle-exact allocation, x87/VME/PVI breadth, global timing refactor, or public ABI change. |
| Reference Baseline | `cb534967`, T360 S1--S3 records, T359 S2--S6 ledgers, T357 S6/S7 ledgers, and the 80286 Appendix-B / 80386 PRM primary editions named by T360 S1. |
| Candidate Proposal | [Four-profile Intel timing source reconciliation](../proposals/m5-four-profile-timing-source-reconciliation.md), bounded S4 contextual-normalization unit. |
| Files And ABI Surface | Evidence/TODO/history/CURRENT, plus only the directly responsible `src/core/machine/machine.c`, its focused timing smoke(s), and CMake registration if a reproduced source-row defect requires it. No public ABI, execution semantics, new elapsed-tick publisher, or unrelated classifier change. |
| Applicable Rules | Task Reading Set; source policy; architecture one-owner/variant rule; coding source-discipline rule; `EXECUTION.md` mechanism/evidence/transfer/similar-issue rules; `DOCUMENT.md` topology rule. |
| Verification | Build a form-by-form 80286/80386 contextual ledger that maps each selected T359 source form to primary citation, source condition, classifier branch and existing proof; search all timing owners for `m`, lexeme, prefixes, EA, odd word, privilege, descriptor, and following-byte branches; reproduce and repair any discrepancy with focused proof; otherwise prove no runtime delta. Run relevant focused smokes, the T360 verifier, documentation governance, diff check, and full current gate. |
| Expected Markers | Indexed S4 context ledger with every selected row classified as exact or transferred; any repair has a focused marker and consumer sweep; no source context is silently assumed; full current gate passes. |
| Asset Needs | Existing primary manual scans and retained evidence only; no imported source, firmware, guest media, or external executable. |
| Reporting Requirements | Report the form/context matrix, every consumer and classifier disposition, any fixed defect with caller/write/fault sweep, exact transfers, source assumptions, and no-L3-overclaim boundary. |
| Stop Conditions | Stop for an unresolvable primary-source ambiguity, any needed shared delivery/bus/device change, a source condition that cannot be captured without pre-execution side effects, or a change outside the named classifier/fixtures; report the exact row and transfer rather than inferring a value. |
| Exit Criteria | Every selected 80286/80386 source allocation has a primary-context disposition; source-required contextual variants are normalized or transferred; no duplicate publisher or profile borrowing appears; focused and full-gate proof passes; unresolved cases have exact TODO/Queue receivers. |
| Original Owner Request | Reconcile every selected four-profile timing row against primary Intel material before it is used by the complete timing corpus or cycle-exact profile; do not average conflicts or infer an answer from a secondary emulator or host benchmark. |
| Similar-Issue Sweep | Search all tracked production timing owners, timing tests, ledgers, Queue and TODO for Appendix-B condition additions, 80386 `m`/lexeme rows, mode/privilege, prefix, EA, odd-word, following-byte, descriptor-granularity, and source-retirement wording; classify every hit as owned S4 context, a distinct physical/delivery receiver, or explicit debt. |

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
| T360 S3 | Accepted the exact non-allocation of 8086/80186 dynamic arithmetic ranges: no source formula exists, both profile routes retain the visible transfer, and one precise formula-or-observation TODO owns later work. [Evidence](../etc/evidence/t360-s3-8086-80186-range-form-disposition.md). |
| T359 | Closed its bounded four-profile instruction-retirement corpus: S1 inventory plus S2--S6 source classifiers use one successful-retirement publisher; S7 verifies all unallocated rows transfer to T360, bus/service, cycle-exact, or x87/VME receivers. 245/245 current-gate tests passed. No L3 or cycle-exact claim is made. [Closure audit](../etc/evidence/t359-s7-complete-instruction-timing-closure-audit.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |

## Recent Governance

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

- **M5 Td S82:** restored all 19 historical NXVM README screenshots as static
  documentation assets; rebuilt the public README around the current CMake
  path and truthful NXVM-first, future-NXVDM boundary; and aligned the current
  design/rule wording and README schema self-test. Documentation self-test,
  default governance check, README-link check, and diff check passed. Td work
  has no runtime or artifact change.
