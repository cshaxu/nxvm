# Project Status

## Current Work

**Active.** M5 T357 S8 performs the final finite-corpus cross-profile closure
audit for instruction-timed execution.

## M5 T357 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved completion of T357 after accepted S1--S7. |
| Objective | Audit and close only the finite source-backed 8086, 80186, 80286, and 80386 instruction-timing corpus allocated by the T357 proposal; verify one successful-retirement publication owner and exact transfers for all non-corpus timing work. |
| Non-goals | Do not add new opcode timing rows, alter instruction semantics, claim memory wait states, prefetch, bus/HOLD/DMA arbitration, device latency, fault/interrupt-cycle timing, x87, VME/PVI, host time, or cycle-exact/L3 completion. |
| Reference Baseline | `91b3fb2d` / `vm-0-5-0357`, accepted T357 S1--S7 evidence, and the selected Intel 8086/80186/80286/80386 manual tables. |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md) and [T357 task record](../history/M5-T357-instruction-timed-execution.md). |
| Files And ABI Surface | Audit evidence, task history, CURRENT, Queue/TODO transfer references if a verified omission requires them, and developer artifact metadata only. No runtime/API/ABI or broad timing-ledger change absent a newly approved corrective packet. |
| Applicable Rules | Apply the Task Reading Set and execution/coding/architecture/documentation/source-policy authorities. Appendix-B selected values control; an actual table/prose conflict must remain a precise TODO revisit. |
| Verification | Reconcile every allocated S1--S7 profile/form/mode row to its evidence and smoke; confirm the sole successful-retirement elapsed-tick publisher, source maximum/budget/fallback/reset/fault boundaries, current-gate, documentation governance, and developer artifact hash. |
| Expected Markers | All focused T357 ledger markers and the full current-gate are green; one closure record identifies the finite corpus and explicit transfers. |
| Asset Needs | Read-only retained Intel manual tables and locally recorded T357 evidence. No external asset enters the repository. |
| Reporting Requirements | Record the row-by-row profile/mode closure, Appendix-B precedence/TODO disposition, artifact command/SHA-256, full-gate result, and every transferred timing category without claiming L3 cycle exactness. |
| Stop Conditions | Stop if an allocated row lacks authority/evidence, an elapsed-time writer or source/config leakage is found, a transfer is unowned, or closure requires runtime repair; return that finding to its mechanism owner rather than closing T357. |
| Exit Criteria | The finite corpus is fully accounted for across four profiles; S3/S6/S7 I/O and VM86 boundaries are reconciled; all non-selected timing remains explicitly transferred; the artifact and gates are verified; no completion language exceeds instruction-timed execution. |
| Original Owner Request | Complete T357 as a profile-aware instruction-form timing prerequisite without claiming bus or cycle-exact timing. |
| Similar-Issue Sweep | Sweep all T357 evidence, source-ledger forms and profile tables, timing consumers, fallback/max/budget paths, elapsed writers, Queue/TODO transfers, and the retained artifact process. |

## Current Technical Baseline

- **Current developer artifact:** T357 selects `vm-0-5-0357` /
  `build/output/nxvm_0_5_0357.exe`; S6 rebuilt it at `b5a0a37f` as
  `E0A9E0343787196A3292978D64AB03B92F9E904CCF7AA88680C376F2EFA08772`.
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
| T357 S7 | Closes the selected 80386 ordinary `IN`/`OUT` protected and permission-map timing rows from Appendix B, including the VM86 TSS I/O-map admission repair shared by ordinary and string I/O. The 80286 `NOP = 3` consumer sweep retains device-clock ratios and repairs only affected expected observables/trace capacity. 240 current-gate tests passed. [S7](../etc/evidence/t357-s7-80386-protected-io-timing.md). |
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
