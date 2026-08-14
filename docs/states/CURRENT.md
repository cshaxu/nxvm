# Project Status

## Current Work

**Active.** M5 T357 S3 establishes the immutable, source-backed 80386 CPU
timing ledger and its single retirement-time consumer for the selected L3 corpus.

## M5 T357 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic L3 implementation on 2026-08-13. T357 S1 and S2 are accepted at `9f869306` and `dcd735a3`; S3 consumes the S1 form contract and the S2 private preview rather than extending synthetic surcharge recognition. |
| Objective | Replace selected 80386 CPU instruction-cost synthesis with one immutable, source-backed ledger and one owner-local classification route at the existing successful-retirement publication boundary. Establish an explicit unavailable/transfer disposition for every form not supported by a cited 80386 row. |
| Non-goals | No guessed cycle values; no per-opcode guest-success patching; no 8086/80186/80286 source timing admission in this S; no public mutable timing table or new product API; no device/port latency, wait-state, HOLD/DMA, prefetch/cache/pin waveform, exact fault/interrupt cycle, x87, host-time, Windows-readiness, or M5-closure claim. |
| Reference Baseline | `dcd735a3` on `master`; accepted [S1 source contract](../etc/evidence/t357-s1-instruction-timing-contract.md), accepted [S2 CPU preview](../etc/evidence/t357-s2-cpu-timing-preview-plan.md), and [instruction-timed-execution proposal](../proposals/m5-instruction-timed-execution.md). |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md). |
| Files And ABI Surface | CPU/machine-private timing owner, existing timing smoke plus one ledger owner smoke, CMake/evidence/CURRENT/history as required. Compatibility configuration/profile composition may be translated once only if no second mutable timing truth remains. No public interface growth, device source, third-party import, or product-boundary change. |
| Applicable Rules | Task Reading Set; accepted T357 S1/S2 evidence; architecture single-owner/variant/one-publication invariant; coding single type/vocabulary and bounded-owner rules; execution mechanism/similar-issue sweep; source policy; T354 deterministic timeline contract. |
| Verification | Record exact 80386 primary source/table locations and values for every admitted form/variant. Prove profile/form selection, taken/not-taken `Jcc` using S2 `m`, prefix and selected string rules, register-versus-memory/port distinctions, stable unavailable disposition, budget/overflow/reset/fault/no-publication, and unchanged deterministic scheduler ordering. Record 8086/80186/80286 as later T357 source-ledger transfers, build `nxvm_0_5_0357.exe`, record SHA-256, and pass current-gates-gcc. |
| Expected Markers | Indexed source-to-ledger table; owner timing-ledger smoke; explicit Jcc next-component proof; no synthetic recognition path; artifact SHA-256; full current-gate result. |
| Asset Needs | Primary Intel manuals as research references only; no guest media, firmware, Bochs/PCjs source import, or external runtime dependency. |
| Reporting Requirements | Deliver one complete implementation P only after every selected 80386 form value, source location, unavailable policy, caller/write/fault sweep, focused proof, artifact, and current gate is complete. Report a source conflict or any required bus/chipset model immediately. |
| Stop Conditions | Stop for owner decision if an exact selected value lacks a primary source, a form requires device/bus timing to be meaningful, compatibility translation creates a second timing truth, or correct integration requires public ABI, generic executor, scheduler, or device redesign. Transfer rather than invent a value. |
| Exit Criteria | Selected source-backed forms use only the immutable ledger at successful retirement; unsupported forms follow a tested stable disposition; no fault/stop/overflow publishes form time; S2 preview is used only where source semantics require `m`; reset/scheduler/device ordering remain deterministic; artifact/evidence/history and full sweep are complete. |
| Original Owner Request | Fully and stably implement high-value devices and complete L3 timing before deciding the next phase, using a holistic mechanism rather than incremental patches. |
| Similar-Issue Sweep | Recheck every `ticks_per_instruction`, `instruction_timing`, `maximum_instruction_ticks`, elapsed-tick writer/reader, profile/composition propagation, CPU retire/trace/scheduler path, selected prefix/branch/string/memory/port form, reset/fault/interrupt/budget branch, timing smoke, CMake gate, evidence, Queue, TODO, and roadmap reference. |

## Current Technical Baseline

- **Current developer artifact:** T357 selects `vm-0-5-0357` /
  `build/output/nxvm_0_5_0357.exe`; S3 rebuilt it as
  `12921803AAC84B617326A832654BB5E27D4C2DF6E016B9D6FCF7278C651999CC`.
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
| T357 S2 | Accepted CPU-private nonpublishing lexical timing preview for the selected 80386 `Jcc` `m` prerequisite; prefix/ModR/M/SIB/displacement/immediate accounting, unavailable boundaries, reset alias, and no-publication proof passed. Artifact SHA-256 `CF8C225A4AD850C6EB9EA79A0C106171B0C0CF7340BE139EC8999F4DAFFA9B8C`; 235 current-gate tests passed. [Evidence](../etc/evidence/t357-s2-cpu-timing-preview-plan.md). |
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
