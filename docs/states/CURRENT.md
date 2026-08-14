# Project Status

## Current Work

**Active.** M5 T359 S3 allocates the source-backed four-profile control and
stack-transfer timing mechanism.

## M5 T359 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved ordered single-agent execution of the M5 Queue through L3 closure on 2026-08-14. T359 S2 was accepted at `21642c2b`; its transfer record and the S1 inventory admit the next mechanism unit. |
| Objective | Replace the one-tick transfer only for source-backed, non-privileged control and stack forms with one private four-profile outcome/stack classifier at the successful-retirement publisher: near/far direct and indirect `CALL`/`JMP`, near `RET`, direct register/memory `PUSH`/`POP`, `PUSHA`/`POPA`, `PUSHF`/`POPF`, `ENTER`/`LEAVE`, `LOOP`/`JCXZ`, `HLT`, and source-backed software `INT`/`INTO`/same-level `IRET` forms where their delivery path has an exact primary row. |
| Non-goals | No string/repeat or ordinary-I/O rows (S4); no 80386 secondary integer/prefix-width matrix (S5); no selector/table/control/debug/task, call-gate, outer-CPL, VM86 transition, NMI, or generic exception-delivery timing (S6 or physical receiver); no x87, physical waits/HOLD/DMA/prefetch/cache/pin timing, device latency, public ABI, or Intel-undefined state timing. |
| Reference Baseline | `21642c2b` / current task artifact `vm-0-5-0359`. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md), [S1 inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md), [S2 ledger](../etc/evidence/t359-s2-four-profile-arithmetic-data-source-ledger.md), and [Queue](QUEUE.md). |
| Files And ABI Surface | Private timing classification in `src/core/machine/machine.c`, existing decoder/lexeme/stack outcome inspection, owner smoke and source ledger evidence, CMake current-gate registration, task history/status, and current artifact. No provider/device/public ABI/generic scheduler or exception-delivery mechanism change. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; one post-refresh successful-retirement publisher; source-selected outcome/stack forms rather than handler-local clocks; exact stack/return publication and defined-state evidence; caller/write/fault sweep; Appendix-B precedence; transfer any privileged or physical dependency rather than infer clocks. |
| Verification | Build an exact Intel source-to-form ledger for all admitted profiles and real/protected/ordinary-VM86 dispositions. Prove direct/indirect, taken/not-taken, return/stack width, operand/address prefix, legal LOCK/prefix, stack data/result, EIP, defined FLAGS, restart/fault zero-cost, preflight maximum, reset/repeat/provider publication, and no admitted successful form reaches the one-tick receiver. Prove every excluded cross-privilege/exception path is transferred. Run focused owner and retained markers, inventory verifier, full current-gate, governance, artifact target/hash, and diff checks. |
| Expected Markers | Existing T357 ledger markers; a new T359 S3 control-stack timing marker; `verify-t359-instruction-timing-inventory`; rebuilt `vm-0-5-0359` artifact. |
| Asset Needs | Intel 8086 Tables 2-20/2-21, iAPX 86/88/186/188 Table 1-16, 80286/80287 PRM Appendix B, and 80386 PRM section 17.2.2.3/table 8-1 and exact instruction pages; project-owned source/tests only. No Bochs/PCjs code, firmware, guest media, or third-party source import. |
| Reporting Requirements | One complete P1 only: source ledger, all admitted mechanism implementation/proof, caller/write/failure sweep, source conflict/physical transfers, current-gate, artifact hash, commit, and push. Do not deliver a single transfer opcode, stack form, or profile as a partial P. |
| Stop Conditions | Stop for owner direction if an admitted form lacks an exact primary row, the row depends on physical or exception-delivery timing, a semantic defect requires a generic stack/exception mechanism change, or an unselected cross-privilege/VM86/table/task transition is needed. Record exact forms and transfer; do not infer clocks. |
| Exit Criteria | Every S3 form has an exact source-backed profile/mode/variant disposition and shares one truthful control/stack outcome mechanism where semantics match; string/repeat/I-O, 80386 secondary, and privileged/physical dependencies are transferred to S4--S7, T360, or later Queue receivers; no one-tick fallback remains for an admitted successful form; all required checks pass. |
| Original Owner Request | Execute the current Queue until a comprehensive L3 timing closure audit, with holistic mechanism planning rather than patch-on-patch clock additions. |
| Similar-Issue Sweep | Search all control-transfer, stack, interrupt-return, HLT, LOOP and group `FF` handlers; metadata/profile gates; prefix/LOCK classifier; stack read/write and instruction-outcome helpers; timing maxima and publisher; current control/stack tests; T357/T359 evidence, Queue, and TODO. Classify each hit as S3-owned, S4--S7, T360 source conflict, or physical receiver. |

## Current Technical Baseline

- **Current developer artifact:** T359 S3 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `AC1A4A30CAB8ED63E436FB52FD4B97F1AA28E6DBB0EDFA593B600B5B61A743A4`.
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
| T359 S2 | Accepted the private, source-backed four-profile arithmetic/data timing classifier at the sole successful-retirement publisher. It covers the admitted ALU, FLAGS, data, Group 3, immediate-IMUL, MOV/LEA and SETcc shapes; distinguishes odd word reads from read-modify-write; raises the source preflight ceiling to the documented 46-clock 80386 IDIV memory row; and transfers range-only 8086/80186 arithmetic rows to T360. The owner marker, strengthened inventory verifier, governance, developer artifact SHA-256, and 241/241 current-gate tests passed. [Evidence](../etc/evidence/t359-s2-four-profile-arithmetic-data-source-ledger.md). |
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
