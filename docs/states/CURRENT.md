# Project Status

## Current Work

**Active.** M5 T360 S5 four-profile timing-source closure audit is admitted in
single-session mode.

## M5 T360 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved continuation of T360 after accepted S4 `814d37cc`, under the approved four-profile source-reconciliation proposal and the owner's direction to complete the queue in single-session mode. |
| Objective | Perform the T360 task-level closure audit: reconcile every S1--S4 source decision, every successful-retirement timing consumer, every unresolved primary ambiguity, and every physical/delivery receiver; decide whether T360 can close without an unsupported L3 or cycle-exact claim. |
| Non-goals | No runtime timing allocation, source import, instruction-semantic repair, bus/device/delivery timing, cycle-exact implementation, x87 execution, VME/PVI breadth, global timing refactor, public ABI change, or Queue reordering in this audit. |
| Reference Baseline | `814d37cc`, T360 S1--S4 records, T359 S1--S7 ledgers, T357 S6/S7 records, the Queue, TODO, and the four Intel primary editions recorded by S1. |
| Candidate Proposal | [Four-profile Intel timing source reconciliation](../proposals/m5-four-profile-timing-source-reconciliation.md), final S5 closure-audit unit. |
| Files And ABI Surface | Evidence/history/CURRENT and, only if an audit fact requires a precise debt correction, TODO. No `src/`, CMake, test target, artifact, or ABI change. |
| Applicable Rules | Task Reading Set; source policy; architecture one-owner/variant rule; `EXECUTION.md` closure, mechanism, evidence, transfer and similar-issue rules; `DOCUMENT.md` authority/topology rule. |
| Verification | Re-read the original proposal and all T360 evidence; inspect each timing owner and the publisher order; cross-map every selected row to primary authority/context/proof or an exact receiver; sweep Queue/TODO/history for duplicate, stale, or unowned timing debt; run the T360 verifier, documentation governance, diff check, and full current gate. |
| Expected Markers | Indexed S5 closure audit; one closed/dispositioned owner for every S1 inventory consumer and every T359 transfer; exact TODO/Queue receivers for NOP conflict, legacy range formulas, system/descriptor context, bus/device/delivery/cycle-exact work; no duplicate timing publisher or L3 overclaim. |
| Asset Needs | Existing primary manual scans and committed project evidence only; no imported source, firmware, guest media, host benchmark, or emulator authority. |
| Reporting Requirements | Record original-request mapping, source/consumer/receiver table, closure decision, exact remaining debt and receiver, actual-change review conclusion, and the explicit non-L3/non-cycle-exact boundary. |
| Stop Conditions | Stop if a selected row lacks a primary authority or exact receiver, if a second timing publisher or hidden runtime allocation is found, if closing would require an implementation change, or if the task boundary conflicts with Queue/TODO topology; report the blocker rather than closing T360. |
| Exit Criteria | Every S1 inventory consumer and T359 transfer has one accurate final disposition; all unresolved source ambiguities and runtime-external timing classes have an exact future receiver; task history is complete; T360 makes no unsupported timing claim; verifier, governance, and full gate pass. |
| Original Owner Request | Reconcile every selected four-profile timing row against primary Intel material before it is used by the complete timing corpus or cycle-exact profile; do not average conflicts or infer an answer from a secondary emulator or host benchmark. |
| Similar-Issue Sweep | Search all tracked timing owners, timing tests, T357/T359/T360 evidence, Queue, TODO, task history, and source-policy records for source authority, source allocation, unallocated transfer, Appendix-B, range, NOP, `m`, descriptor, delivery, physical wait, device service, cycle-exact, and L3 wording; classify every hit as closed T360 evidence or one named later receiver. |

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
| T360 S4 | Accepted the 80286/80386 source-context audit: all selected rows retain one successful-retirement publisher; unallocatable system/descriptor contexts transfer exactly, and no timing value or runtime path changed. [Evidence](../etc/evidence/t360-s4-80286-80386-contextual-source-disposition.md). |
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
