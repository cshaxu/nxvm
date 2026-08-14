# Project Status

## Current Work

**Active.** M5 T357 S2 establishes the CPU-owned, side-effect-free timing
preview required to classify source-backed taken-branch timing without a
second execution decoder.

## M5 T357 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic L3 implementation on 2026-08-13. T357 S1 was accepted at `9f869306`; its source-backed `Jcc` requirement exposed this prerequisite. The owner direction is implemented as a bounded S2 mechanism first, followed by a timing-ledger S. Full-gate execution may repair a directly reproduced baseline reset/fixture contract only when that repair is local, evidence-backed, and does not broaden timing semantics. |
| Objective | Add one CPU-owned, private, side-effect-free preview that reports the Intel `m` component count for the next architecturally fetchable instruction. It must let taken-branch timing consume CPU lexical-format truth without executing, publishing, tracing, faulting, or mutating that instruction. |
| Non-goals | No timing ledger or synthetic-cost replacement yet; no guessed branch value; no public timing API, second execution/semantic decoder, CPU semantic repair except a directly reproduced baseline gate contract, wait-state/bus/HOLD/DMA model, prefetch/cache/pin waveform model, exact interrupt/fault cycles, x87 timing, host time, or M5 closure. |
| Reference Baseline | `9f869306` on `master`; accepted [S1 timing contract](../etc/evidence/t357-s1-instruction-timing-contract.md), in particular the 80386 `Jcc` `7+m` row, current CPU fetch/decode route, and T354 selected-L3 ordering. |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md); [T357 S1 contract](../etc/evidence/t357-s1-instruction-timing-contract.md). |
| Files And ABI Surface | `src/core/machine/cpu.c`, `cpu_instructions.c`, private CPU declarations, `machine.c`, owner smoke/CMake/evidence/history/CURRENT as necessary; directly reproduced descriptor no-IDT fixture repairs are limited to their existing owner smokes. `machine.c` may consume a private preview only after the preview contract is proven. No `machine_interface.h`, VM/profile composition, device source, public mutable table, third-party import, or product-boundary change. |
| Applicable Rules | Task Reading Set; S1 contract; architecture single-owner/variant/one-publication invariant; coding single type/vocabulary and bounded-owner rules; execution mechanism/similar-issue sweep; source policy; T354 deterministic timeline contract. |
| Verification | Map all CPU instruction-fetch/decode/length and linear-memory-preview routes. Prove prefix, ModR/M, SIB, displacement and immediate component accounting; valid and inaccessible next-instruction boundaries; no CPU/register/memory/trace/diagnostic/timeline/device publication; default PC/AT reset-vector alias determinism; source-compatible selected taken-`Jcc` proof; and any corrected no-IDT fixture at its real terminal boundary. Build the current developer artifact `nxvm_0_5_0357.exe`, record SHA-256/runtime identity, and run full current-gates-gcc. |
| Expected Markers | CPU timing-preview owner smoke marker; an exact taken-`Jcc` component proof; preview source/caller/write/fault ledger; artifact SHA-256 record; full current-gate result. |
| Asset Needs | Primary manual text only as external research reference; no guest media, firmware, Bochs/PCjs source import, or external runtime dependency. |
| Reporting Requirements | Report one complete implementation P only after every preview route, nonpublication boundary, taken-`Jcc` proof, sweep, artifact, and current-gate is complete. Report a material source/architecture conflict immediately; do not publish internal batches. |
| Stop Conditions | Stop for owner decision if a correct preview requires a second independent decoder, execution/semantic side effect, observable memory/transaction/trace mutation, public CPU API expansion, or a generic CPU-executor or device scheduler redesign. Transfer instead of guessing. |
| Exit Criteria | The CPU owns exactly one preview route whose component result is sufficient for 80386 `Jcc` `m`; all previewed fetch/decode variants have no guest or diagnostic publication; inaccessible input has an explicit nonpublishing result; the machine has no duplicate execution/semantic parser; deterministic reset/full gate remain green; artifact/evidence/history and similar-issue sweep are complete. |
| Original Owner Request | Fully and stably implement high-value devices and complete L3 timing before deciding the next phase, using a holistic mechanism rather than incremental patches. |
| Similar-Issue Sweep | Recheck all CPU fetch/decode/length/prefix/ModR-M/SIB/displacement/immediate, `ExecInit`/`ExecIns`, linear memory preview/read, fault/diagnostic/trace/transaction, debug, reset, and `Jcc` references. Enumerate every production hit as preview owner, consumer, not applicable, or explicitly transferred. |

## Current Technical Baseline

- **Current developer artifact:** T357 selects `vm-0-5-0357` /
  `build/output/nxvm_0_5_0357.exe`; S2 must build and record its SHA-256
  before acceptance.
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
| T357 S1 | Accepted the source-backed instruction-time contract: one CPU retirement publication point, all current synthetic-cost paths inventoried, primary 8086--80386 reference assumptions recorded, selected form corpus allocated, and bus/cycle-exact boundaries transferred. [Evidence](../etc/evidence/t357-s1-instruction-timing-contract.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |

## Recent Governance

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

- **M5 Td S81:** withdrew the duplicate 80286 `LOCK` candidate after the
  T339 closure audit reconfirmed that accepted T328 already owns the complete
  pre-386 prefix-policy matrix. The Queue now advances directly from T339 to
  the 80386DX form closure; current proposals and T339 transfer ledgers name
  T328, T341, and T342 truthfully. Documentation governance and diff check
  passed. Td work has no runtime or artifact change.

- **M5 Td S80:** reordered the four-profile CPU-completeness Queue into
  dependency-bounded 8086/80186, 80286 descriptor-transfer, 80286 `LOCK`,
  80386DX form, 80386DX state, 80386DX audit, and cross-profile closure
  candidates; added the corresponding unnumbered proposals. Documentation
  governance and diff check passed. Td work has no runtime or artifact change.

- **M5 Td S79:** reordered the M5 CPU-completeness program around a
  four-profile audit, shared delivery foundations, 8086/80186, 80286, and
  80386DX closure candidates, then cross-profile verification. Each candidate
  has a linked unnumbered proposal; no implementation task was allocated.
  Documentation governance, Queue-link verification, and diff check passed.

- **M5 Td S77:** moved the closed T323/T325/T328 proposals into matching
  history companions; made every current proposal require a Queue link and
  added the orphan-proposal negative self-test; compacted repeated
  mechanism-defect requirements into role-specific authorities; clarified
  historical terminology retention and the idle T332 artifact wording.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.
