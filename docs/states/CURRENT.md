# Project Status

## Current Work

**Active.** M5 T359 S2 allocates the source-backed four-profile arithmetic,
FLAGS, data, and ModRM/EA timing mechanism.

## M5 T359 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved ordered single-agent execution of the M5 Queue through L3 closure on 2026-08-14. T359 S1 was accepted at `498838f8`; its complete receiver inventory admits this first implementation mechanism. |
| Objective | Replace the visible one-tick transfer only for primary arithmetic, defined-FLAGS, data, and ModRM/EA forms with private, immutable, four-profile primary-source classifications at the one successful-retirement publisher. Cover exact primary ALU forms `00`--`3D`, groups `80/81/83`, TEST/XCHG, adjustment and conversion forms, INC/DEC, immediate IMUL, Group 3 arithmetic, byte INC/DEC, selected register/memory/moffs/immediate MOV, LEA, and 80386 `0F 90`--`9F` SETcc where one shared data/FLAGS/EA mechanism is real. |
| Non-goals | No call/return/interrupt/LOOP/Jcc breadth, string/repeat, ordinary I/O, privileged `0F`, selector/table/task/debug timing, x87 execution, physical waits/HOLD/DMA/prefetch/cache/pin timing, device latency, fault/interrupt delivery clocks, source import, or public timing ABI. Do not assign a timing value to an Intel-undefined FLAGS result. |
| Reference Baseline | `498838f8` / current task artifact `vm-0-5-0359`. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md), [S1 inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md), and [Queue](QUEUE.md). |
| Files And ABI Surface | Private source-cost classification in `src/core/machine/machine.c`, existing CPU metadata/decoder/lexeme inspection, owner timing smoke(s), source ledger evidence, CMake current-gate registration only if a new owner smoke is needed, task history/status, and current artifact. No provider, device, public ABI, or generic scheduler change. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; one post-refresh successful-retirement publisher; profile-local primary-source values; shared classification rather than per-opcode patches; defined-state-only assertions; exact validation-to-publication and source/caller sweep; Appendix-B precedence with contradictions transferred to T360. |
| Verification | Build a source-to-form table with exact Intel table/page/row and assumptions for every admitted form/profile/mode. Prove register/memory/ModRM/EA, operand/address width, legal prefix/LOCK, real/protected/ordinary-VM86 disposition where a source row exists, defined FLAGS, result/publication, fault/restart zero-cost, preflight maximum, reset/repeat state, and one elapsed-tick/provider publication. Mechanically prove no admitted form still falls through to the one-tick receiver; run focused owner markers, inventory verifier, full current-gate, governance, artifact target/hash, and diff checks. |
| Expected Markers | Existing T357 ledger markers; a new T359 S2 arithmetic/data timing marker; `verify-t359-instruction-timing-inventory`; rebuilt `vm-0-5-0359` artifact. |
| Asset Needs | Intel 8086 Tables 2-20/2-21, iAPX 86/88/186/188 Table 1-16, 80286/80287 PRM Appendix B, and 80386 PRM section 17.2.2.3 or exact instruction tables; project-owned source/tests only. No Bochs/PCjs code, firmware, guest media, or third-party source import. |
| Reporting Requirements | One complete P1 only: exact source ledger and assumptions, full admitted mechanism implementation/proof, caller/write/failure sweep, source conflict/physical transfers, current-gate, artifact hash, commit, and push. Do not deliver one opcode family, one profile, or one focused clock as a partial P. |
| Stop Conditions | Stop for owner direction if an admitted profile/form lacks an exact primary row, table assumptions require physical timing, a source conflict lacks the already-approved Appendix-B precedence, a semantic defect is required outside the shared classification owner, or an unselected control/string/privileged mechanism is needed. Record exact forms and transfer; do not infer clocks. |
| Exit Criteria | Every S2 form listed in Objective has an exact source-backed profile/mode/variant disposition and shares one truthful cost mechanism where semantics match; every non-source or physical dependency is transferred to S3--S6, T360, or later Queue receiver; no one-tick fallback remains for an admitted successful form; all required checks pass. |
| Original Owner Request | Execute the current Queue until a comprehensive L3 timing closure audit, with holistic mechanism planning rather than patch-on-patch clock additions. |
| Similar-Issue Sweep | Search all primary ALU/data/group handlers, `0F 90`--`9F`, metadata gates, prefix/LOCK classifier, ModRM/EA and operand-width helpers, all source-cost cases/ledger entries, timing maxima, elapsed publisher, profile tests, T357/T359 evidence, Queue, and TODO. Classify each hit as S2-owned, later S3--S6, T360 source conflict, or physical receiver. |

## Current Technical Baseline

- **Current developer artifact:** T359 S2 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `2D91DCD23488FD86E64B1EB5126C27DE293183619F7841C96E447FBA63E15904`.
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
| T359 S1 | Accepted a mechanically checked 256-primary/256-secondary four-profile timing inventory. Every executable timing form now has a selected row, receiver, external boundary, or rejection; S2--S6 are dependency-ordered by mechanism. The one-tick unallocated fallback remains a transfer, not an Intel timing claim. [Evidence](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |

## Recent Governance

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

- **M5 Td S81:** withdrew the duplicate 80286 `LOCK` candidate after the
  T339 closure audit reconfirmed that accepted T328 already owns the complete
  pre-386 prefix-policy matrix. The Queue now advances directly from T339 to
  the 80386DX form closure; current proposals and T339 transfer ledgers name
  T328, T341, and T342 truthfully. Documentation governance and diff check
  passed. Td work has no runtime or artifact change.
