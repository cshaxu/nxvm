# Project Status

## Current Work

**Active.** M5 T357 S7 allocates the remaining selected 80386 port-I/O timing
rows and repairs the shared VM86 permission-map admission defect they expose.

## M5 T357 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved T357 continuation, expanded on 2026-08-13 after reproducing the shared VM86 I/O-permission defect: S7 closes only the selected 80386 ordinary `IN`/`OUT` mode rows explicitly transferred by S3, and repairs their one shared permission-map owner for VM86. |
| Objective | Allocate the 80386 Appendix-B/PRM successful timing rows for all selected immediate/DX `IN`/`OUT` forms at the existing post-refresh retirement publisher, and make VM86 consult the existing TSS I/O permission map through that same owner. |
| Non-goals | Do not allocate near/`JCXZ` branches, other string forms, arbitrary CPU timing rows, memory wait states, prefetch, bus/HOLD/DMA, device latency, fault/interrupt cycles, x87, VME/PVI, host time, or Windows behavior. These are not in T357's selected finite corpus and remain explicit later timing-corpus or physical-timing transfers. |
| Reference Baseline | `27478a6b` / `vm-0-5-0357`, accepted S1--S6 evidence, and Intel 80386 PRM `IN`/`OUT` opcode tables. |
| Candidate Proposal | [Instruction-timed execution](../proposals/m5-instruction-timed-execution.md) and [T357 task record](../history/M5-T357-instruction-timed-execution.md). |
| Files And ABI Surface | Private 80386 timing classification; the existing private TSS I/O-permission owner; owner smoke/target and focused existing I/O-string smoke; evidence/index/current/history only. No public ABI, provider ABI, TSS layout, second permission owner, or second time writer. |
| Applicable Rules | Apply Task Reading Set and execution/coding/architecture/documentation/source-policy authorities. Preserve one post-refresh/pre-publication elapsed-tick owner, one I/O-permission owner, and permission/fault atomicity. Where Appendix B gives a selected timing row it controls; record any conflicting prose for later TODO review. |
| Verification | Prove immediate/DX `IN`/`OUT` selected real, protected `CPL <= IOPL`, protected permission-map, and ordinary VM86 permission-map success rows; prove ordinary and string I/O VM86 bitmap permit/deny through the same owner; exact source costs, provider result, no selected timing on denied faults, fallback isolation, reset/budget/overflow/deterministic publication, and full current-gate. |
| Expected Markers | A focused `M5:T357:S7:80386-PROTECTED-IO-TIMING:OK` marker and full current-gate remain green. |
| Asset Needs | Read-only Intel 80386 PRM `IN` and `OUT` tables. No external material enters the repository. |
| Reporting Requirements | Record Appendix-B/PRM real/`pm` source rows, CPL/IOPL and permission-map classifications, the VM86 defect/reproducer and ordinary/string caller sweep, selected/unselected boundary, commands/results, artifact, and actual-change review before closure. |
| Stop Conditions | Stop if repair requires a TSS-layout, provider-ABI, generic fault-delivery, or second time-writer/public-contract change; if a successful source row cannot be distinguished from permission validation; or if Appendix B lacks the selected row and conflicts with the main text. Record the latter as TODO and transfer rather than guess. |
| Exit Criteria | Every selected real/protected/ordinary-VM86 successful `IN`/`OUT` class is source-backed and focused-proven; denied cases publish no successful cost; S3's selected-I/O transfer is closed; later non-selected branch/string/CPU rows are explicitly transferred; no selected value derives from compatibility configuration. |
| Original Owner Request | Complete T357 as a profile-aware instruction-form timing prerequisite without claiming bus or cycle-exact timing. |
| Similar-Issue Sweep | Sweep all 80386 selected port opcodes, real/protected/VM86 classification, post-refresh timing selection, I/O permission callers, maximum/budget paths, elapsed writers/readers, current timing smokes, and every S3 selected-I/O transfer. |

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
| T357 S6 | Latest accepted T357 progress replaces 80286 compatibility cadence with selected Appendix-B `NOP`, `MOV`, `MOVSB`/`REP`, Jcc and `IN`/`OUT` rows. It retains distinct 80286 base-index-displacement/odd-word construction, fallback and publication boundaries; Appendix B controls the `NOP = 3` allocation while the conflicting prose is a later TODO. Artifact SHA-256 `E0A9E0343787196A3292978D64AB03B92F9E904CCF7AA88680C376F2EFA08772`; 239 current-gate tests passed. [S6](../etc/evidence/t357-s6-80286-instruction-timing-ledger.md). |
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
