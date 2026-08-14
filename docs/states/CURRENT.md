# Project Status

## Current Work

**Active.** M5 T359 S6 privileged-form timing and corpus reconciliation is
admitted in single-session mode.

## M5 T359 S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved M5 L3 timing program; T359 is the latest open numeric task and S6 is the next unused subtask after accepted S5. Scope is the T359 proposal's ordered S6 unit. |
| Objective | Reconcile every T359 80286/80386 privileged, selector/table, control/debug/test, task, FS/GS and VM86 timing receiver into either a source-backed successful-retirement row owned by one private classifier or a precise T360 transfer. |
| Non-goals | No semantic CPU repair; no exception/IRQ/NMI/task/VM86 delivery-cycle allocation; no bus, wait/HOLD, prefetch/cache, x87, host-time, or Windows claim; no inferred midpoint for incomplete/conflicting source. |
| Reference Baseline | `fa02c334fe3366c25e4e5761673f011f0c1e8eb4`, `vm-0-5-0359`, and accepted S1--S5 inventory/history evidence. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md), ordered unit S6; primary authority is the Intel 80286/80386 programmer reference material recorded by the source ledger. |
| Files And ABI Surface | Expected: `src/core/machine/machine.c`, a T359 S6 owner smoke and CMake registration, `cmake/verify_t359_instruction_timing_inventory.cmake`, the S1 inventory, new indexed S6 evidence, T359 history, and this packet. No public ABI or decoder/handler ownership change without a packet revision. |
| Applicable Rules | `docs/rules/ARCHITECTURE.md` sole mutable timing owner and shared-variant publication; `docs/rules/CODING.md` C11/project types and owner-local helper; `docs/rules/EXECUTION.md` source/evidence, actual-change review, artifact and closure rules; source policy applies to external manual research. |
| Verification | Mechanically enumerate all S6 receivers against dispatch/metadata; ledger every row with source page/form, classifier disposition, mode/prefix/fault boundary and T360 receiver where not allocated; focused source-timing smoke; timing inventory verifier; documentation governance; diff check; full current-gate. Rebuild/copy the T359 artifact only if a runnable path changes. |
| Expected Markers | `M5:T359:S6:PRIVILEGED-TIMING:OK`; `T359 four-profile instruction timing inventory passed.`; full current-gate passes. |
| Asset Needs | No guest media, firmware, external executable, source import, or runtime dependency. Manual pages are research/provenance only. |
| Reporting Requirements | Record the primary page/form source, all classifier callers and success/fault publication boundary, the per-receiver allocation or T360 transfer, focused and full-gate result, and any source conflict. Report a material source/ownership conflict before broadening scope. |
| Stop Conditions | Stop and transfer if a numeric row depends on exception/IRQ/NMI/task/VM86 delivery, a physical transaction/wait/prefetch/cache contract, an unmodelled descriptor or paging outcome, or a source conflict requiring a new source policy decision. Do not alter decoder, semantic handlers, shared delivery, task, or bus owners without revised approval. |
| Exit Criteria | Every S6 receiver named by the S1 inventory has a source-backed classifier row with focused evidence or an exact T360 transfer; no successful rejected/fault/delivery path receives invented instruction time; one publisher remains; all required gates pass; history and index preserve the source ledger. |
| Original Owner Request | Execute the current Queue through a comprehensive source-backed L3 timing model, then perform the L3 closure audit; keep the work holistic and do not replace manual authority with convenient opcode batches. |
| Similar-Issue Sweep | Sweep all `0F` system dispatches, FS/GS and full-pointer secondary forms, control/debug/test moves, selector/table/task forms, and their real/protected/ordinary-VM86/prefix/LOCK success versus zero-publication outcomes; compare every receiver with S1 inventory and transfer any delivery-dependent variant together. |

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
| T359 S5 | Accepted one private 80386 secondary-integer classifier at the successful-retirement publisher. It allocates near `Jcc`, bit test/modify, double shifts, two-operand `IMUL`, bit scan, and `MOVZX`/`MOVSX`; retains S2 `SETcc`; lifts the correct maximum preflight to 106 for 32-bit zero `BSR`; and transfers privileged forms, physical timing, and source conflicts. [Evidence](../etc/evidence/t359-s5-80386-secondary-source-ledger.md). |
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
