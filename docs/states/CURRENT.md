# Project Status

## Current Work

**Active.** M5 T357 S4 allocates the selected 8086 source-timing ledger
(Single-Session Mode).

## M5 T357 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved completion of T357 on 2026-08-13. S4 is the next bounded source-ledger packet under accepted M5 T357, limited to the 8086 rows explicitly listed below. No rule exception is approved. |
| Objective | Replace the retained synthetic compatibility cadence for the selected 8086 corpus with immutable, source-backed 8086 form costs at the one existing CPU-retirement publication point. Preserve explicit fallback for every unallocated 8086 form. |
| Non-goals | Do not allocate 80186, 80286, or further 80386 rows; do not model 8088, prefetch depletion, wait states, HOLD/DMA arbitration, device latency, exact exception/IRQ cycles, bus phases, cache, x87, host time, or Windows behavior. Do not modify `docs/rules/`. |
| Reference Baseline | `4afb2847` / `vm-0-5-0357`, with accepted S1--S3 evidence in `docs/etc/evidence/t357-s1-instruction-timing-contract.md`, `t357-s2-cpu-timing-preview-plan.md`, and `t357-s3-80386-instruction-timing-ledger.md`. |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md), [T357 task record](../history/M5-T357-instruction-timed-execution.md), and the retained Intel 8086 reference identified by S1. |
| Files And ABI Surface | Expected changes are private `src/core/machine/machine.c` timing ownership, an 8086 owner smoke and its target registration, T357 evidence/index/current/history facts as needed. No public ABI, CPU execution API, device contract, or production timing writer may be added. |
| Applicable Rules | Read and apply the Task Reading Set, `docs/rules/EXECUTION.md`, `CONTRIBUTING.md`, architecture/coding authorities, documentation rules at closure, and the source/research policy. One elapsed-tick writer and post-refresh/pre-publication timing boundary remain mandatory. |
| Verification | Prove exact 8086 values for `NOP`, `CLC`, immediate/register/direct-memory/moffs `MOV`, single and bounded `REP MOVSB`, short `Jcc` taken/not-taken, immediate/DX `IN`/`OUT`, one single segment-override memory `MOV`, and odd-word additions where the chosen source row requires them. Prove selected values ignore legacy timing configuration; unallocated 8086 forms retain a stable explicit fallback; selected faults/stops publish no cost; reset/budget/overflow and scheduler/provider publication remain deterministic. Configure, focused smoke, documentation governance, diff check, and full current-gate must pass. |
| Expected Markers | New owner smoke marker `M5:T357:S4:8086-INSTRUCTION-TIMING-LEDGER:OK`; all existing timing markers and the full current-gate remain green. |
| Asset Needs | Intel 8086 Family User's Manual, Table 2-20 and Table 2-21 only as a read-only primary research source. No external source, firmware, guest media, or local path enters the repository. |
| Reporting Requirements | Before implementation, confirm the source rows and classification boundary or report a material objection. The implementation P includes source-to-form evidence, exact commands/results, changed surface, similar-issue sweep, artifact result, and explicit transfer of unallocated rows. This single-session continuation documents its actual-change review and does not claim independent review before governance closure. |
| Stop Conditions | Stop for owner direction if a selected value depends on an unmodelled prefetch, bus, device, or interrupt/fault-delivery condition rather than the cited source row; if producing it requires a second elapsed-time writer or a public mutable ledger; or if source evidence cannot distinguish the selected 8086 row. Transfer that row rather than guessing. |
| Exit Criteria | The listed 8086 rows have cited source values and executable focused proof; the one private owner selects them only for the 8086 profile after successful refresh; no selected 8086 value derives from compatibility configuration; selected failure/no-publication and reset/budget/overflow proofs pass; all unallocated 8086 forms have a stable visible transfer policy; full current-gate and closure audit pass; 80186/80286 and remaining 80386 rows are explicitly transferred. |
| Original Owner Request | Complete T357, which establishes profile-aware instruction-form guest time as the physical-L3 prerequisite without misrepresenting bus or cycle-exact timing. |
| Similar-Issue Sweep | Sweep all `cpu_profile` timing branches, source-ledger lookup paths, `maximum_instruction_ticks` derivation, legacy timing configuration paths, elapsed-tick writers/readers, and selected timing smokes. Every early-profile timing branch is either S4-owned, explicitly retained as compatibility pending its own primary source, or transferred to later T357 S evidence. |

## Current Technical Baseline

- **Current developer artifact:** T357 selects `vm-0-5-0357` /
  `build/output/nxvm_0_5_0357.exe`; S4 rebuilt it as
  `DCAA7143AF6A53D75FA3F3FD72E8BA65682C95DAC539747A71E8AF0BCC90E926`.
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
| T357 S3 | Latest accepted T357 progress: S1 timing contract, S2's nonpublishing 80386 `Jcc` lexical `m` preview, and S3's immutable ledger for selected NOP/CLC/MOV/MOVSB/REP MOVSB, short Jcc and real-mode IN/OUT forms. S3 adds the zero-tick delivered-exception boundary, deterministic scheduler integration, and explicit unsupported-form transfers. Artifact SHA-256 `12921803AAC84B617326A832654BB5E27D4C2DF6E016B9D6FCF7278C651999CC`; 57 static gates and 236 current-gate tests passed. [S1](../etc/evidence/t357-s1-instruction-timing-contract.md), [S2](../etc/evidence/t357-s2-cpu-timing-preview-plan.md), [S3](../etc/evidence/t357-s3-80386-instruction-timing-ledger.md). |
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
