# Project Status

## Current Work

**Active.** M5 T359 S5 allocates the source-backed 80386 secondary-integer and
remaining prefix/width timing mechanism.

## M5 T359 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved ordered single-agent execution through the M5 L3 closure audit. T359 S4 was accepted at `0612d4ca`; the approved timing-corpus proposal allocates S5 next. |
| Objective | Complete the exact primary-source disposition and successful-retirement timing mechanism for admitted 80386 non-privileged secondary integer forms: near `Jcc`, `SETcc`, bit test/modify, double shifts, two-operand `IMUL`, bit scan, and `MOVZX`/`MOVSX`; reconcile their legal `66`/`67`/segment/LOCK lexical and operand/address-width variants without duplicating S2 or S4 owners. |
| Non-goals | No S4 string/repeat/ordinary-I/O rows; no table/selector/control/debug/task/VM86 privileged corpus (S6); no generic exception/IRQ delivery, physical bus/I/O service, waits, HOLD/DMA/prefetch/cache/pin timing, x87, public ABI, or Intel-undefined FLAGS timing. |
| Reference Baseline | `0612d4ca` / current task artifact `vm-0-5-0359`. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md), [S1 inventory](../etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md), [S2 ledger](../etc/evidence/t359-s2-four-profile-arithmetic-data-source-ledger.md), [S4 ledger](../etc/evidence/t359-s4-four-profile-string-io-source-ledger.md), and [Queue](QUEUE.md). |
| Files And ABI Surface | Private source classifier/state only at `src/core/machine/machine.c` if required; existing decode/lexeme/operand outcome inspection; owner smoke, source ledger, CMake current-gate registration, task history/status, and current artifact. No CPU decoder, provider/device/public ABI, generic scheduler, or exception-delivery change absent an approved revised packet. |
| Applicable Rules | Task Reading Set, execution, architecture, coding, documentation, and source/research policy; one post-refresh successful-retirement publisher; 80386 PRM source precedence; source-selected lexical/operand outcomes rather than handler-local clocks; defined-state evidence; caller/write/fault sweep; reuse S2/S4 mechanisms where semantics match; transfer any privileged, physical, or source-incomplete dependency rather than infer clocks. |
| Verification | Build a form-to-source ledger for every assigned `0F` integer opcode/form and its accepted 80386 real/protected/ordinary-VM86 disposition. Prove taken/not-taken, register/memory and immediate/count shapes, legal operand/address/segment/LOCK variants, dynamic result-selected rows, defined EIP/GPR/FLAGS, fault/restart zero cost, preflight maximum, reset/provider publication, and that no admitted successful form reaches the one-tick receiver. Reuse retained S2/S4 proof only where the shared outcome mechanism and source row are identical. Run focused owner and retained markers, inventory verifier, full current-gate, governance, artifact target/hash, and diff checks. |
| Expected Markers | Existing T357/T359 timing markers; a new T359 S5 secondary-integer timing marker; `verify-t359-instruction-timing-inventory`; rebuilt `vm-0-5-0359` artifact. |
| Asset Needs | Intel 80386 PRM section 17.2.2.3 / timing table and exact instruction pages, plus the project-owned source/tests and prior T359 ledgers. No Bochs/PCjs code, firmware, guest media, or third-party source import. |
| Reporting Requirements | One complete P1 only: source ledger, all admitted mechanism implementation/proof, caller/write/failure sweep, retained-owner and physical/source-conflict transfers, current-gate, artifact hash, commit, and push. Do not deliver one `0F` opcode, one prefix, a lookup, registration, or status update as a partial P. |
| Stop Conditions | Stop for owner direction if an assigned form lacks an exact primary row, depends on delivery or physical timing, needs a generic lexical/decoder/exception mechanism change, or requires privileged/cross-mode state not selected here. Record exact forms and transfer; do not infer clocks or widen a shared owner silently. |
| Exit Criteria | Every assigned S5 form has an exact source-backed profile/mode/variant disposition and shares a truthful secondary/prefix-width outcome mechanism where semantics match; S4, S6--S7, T360, and later physical receivers own every excluded dependency; no one-tick fallback remains for an admitted successful form; all required checks pass. |
| Original Owner Request | Execute the current Queue until a comprehensive L3 timing closure audit, with holistic mechanism planning rather than patch-on-patch clock additions. |
| Similar-Issue Sweep | Search every 80386 secondary dispatch/handler, metadata/profile gate, prefix/LOCK and operand/address lexical path, dynamic-result helper, timing ledger/max/publisher, current `0F` and prefix tests, T357/T359 evidence, Queue, and TODO. Classify each hit as S5-owned, retained S2/S4, S6--S7, T360 source conflict, or physical receiver. |

## Current Technical Baseline

- **Current developer artifact:** T359 S4 `vm-0-5-0359` /
  `build/output/nxvm_0_5_0359.exe` as
  `D8422AE25FF8608282FAD66E057F76F5BC56143377AAAF4210BF4396A7B9FD89`.
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
| T359 S4 | Accepted one private four-profile string/repeat/ordinary-I/O classifier at the successful-retirement publisher. It allocates all cited primitive and defined REP rows for `MOVS`, `CMPS`, `STOS`, `LODS`, `SCAS`, `INS`, `OUTS`, and ordinary `IN`/`OUT`; preserves reset/continuation and 80386 protected/permission/VM86 I/O outcomes; and transfers only 80286 `REP LODS`, physical service, exceptions, and later privileged work. [Evidence](../etc/evidence/t359-s4-four-profile-string-io-source-ledger.md). |
| T358 | Closed cross-mode mechanism coherence: reconciled I/O permission, exception/IRQ entry, selector/task/return transition, paging/linear access, and prefix/LOCK/width/string classifiers; repaired 80286 delivery, LOCK group legality, and repeated-width preview divergence; passed 240 current-gate tests. [Closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md). |
| T357 | Closed finite source-backed 8086/80186/80286/80386 instruction timing at one successful-retirement publisher: selected form ledgers, 80386 protected/VM86 ordinary I/O, Appendix-B `NOP = 3` consumer reconciliation, 240 current-gate tests, and artifact SHA-256 `83A249015C03CF875896A440D5B43A430C2D875A63F0BE4B9709014A753CAD95`. Unselected forms, waits, physical bus/prefetch, device latency and cycle-exact work remain explicit transfers. [Closure audit](../etc/evidence/t357-s8-cross-profile-closure-audit.md). |
| T356 | Closed the M5 closure audit with an explicit M5-open decision: all selected device owners are evidenced, but instruction timing, bus availability/wait states and selected-profile cycle exactness remain ordered implementation prerequisites; corpus-gated peripheral, storage and product boundaries remain explicit transfers. [History](../history/M5-T356-m5-closure-audit.md). |
| T355 | Closed the bounded Windows 3.x readiness map: an opt-in BYOB HDD/INT13 checkpoint passed, all four probes remain host-observation/non-current, T347/T354 storage/L3 is a verified prerequisite, and all display, input, NMI, physical-timing, Setup, and compatibility breadth remains explicitly transferred. [History](../history/M5-T355-windows-3x-readiness-map.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; its recorded developer artifact and SHA-256 remain in task history; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |

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
