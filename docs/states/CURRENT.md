# Project Status

## Current Work

**Active.** M5 T359 S7 complete instruction-timing corpus closure audit is
admitted in single-session mode.

## M5 T359 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner-approved single-agent completion of the current Queue through its L3 closure audit; T359 S6 was accepted at `11445dc8`, and the approved proposal assigns S7 as the task's final audit unit. |
| Objective | Audit the complete T359 four-profile instruction-timing corpus: prove every S1 receiver has either an exact source-backed successful-retirement allocation at the sole publisher or an exact named T360/cycle-exact/bus transfer, then close T359 without claiming unallocated timing as complete. |
| Non-goals | No new instruction clock, semantic CPU change, source reconciliation, exception/IRQ/NMI/task/VM86 delivery-cycle allocation, physical bus/wait/prefetch/cache service, x87, host-time, Windows, or Queue reordering. |
| Reference Baseline | `11445dc8`, `vm-0-5-0359`, T359 S1--S6 history, the four source ledgers, and the ordered Queue. |
| Candidate Proposal | [Complete instruction-timing corpus](../proposals/m5-complete-instruction-timing-corpus.md), ordered unit S7; [Four-profile Intel timing source reconciliation](../proposals/m5-four-profile-timing-source-reconciliation.md) is the required handoff receiver. |
| Files And ABI Surface | Audit-only: T359 closure history, indexed closure evidence, source inventory/ledger references, CURRENT, and any exact Queue/TODO link correction needed to repair a broken transfer. No `src/`, CMake target, artifact, or public ABI change. |
| Applicable Rules | Task Reading Set; `EXECUTION.md` audit-only/closure evidence rules; `DOCUMENT.md` history/index topology; `ARCHITECTURE.md` one successful-retirement timing publisher and explicit transfer boundary; source policy for provenance references only. |
| Verification | Mechanically reconcile each S1 primary and secondary receiver against S2/S4/S5/S6 ledgers and source classifier anchors; scan for unauthorized one-tick/fallback ownership; verify every remaining class names T360, a later cycle-exact receiver, or a bus/service receiver; run timing inventory and documentation governance gates plus diff check. |
| Expected Markers | `T359 four-profile instruction timing inventory passed.`; documentation governance passes; a closure audit records the exact T360 and later transfer set. |
| Asset Needs | No source import, guest media, firmware, external executable, or runtime dependency. |
| Reporting Requirements | Publish an indexed S7 closure audit that names the selected allocations, source conflicts, physical and delivery transfers, one-publisher verification, exact gates, and the no-overclaim decision. Report a missing/ambiguous receiver before closing rather than manufacturing a row. |
| Stop Conditions | Stop T359 closure if a receiver is neither allocated nor precisely transferred, if an allocation has no primary-source evidence, if a second timing publisher exists, or if closure would require source/semantic work. Admit that work only in the proper T360 or later receiver. |
| Exit Criteria | Every S1 receiver is accounted for by an exact allocation or named future receiver; S2/S4/S5/S6 ledgers remain mutually non-overlapping; the sole publisher and fallback policy are verified; closure history/index and Queue handoff are complete; all audit gates pass. |
| Original Owner Request | Execute the Queue toward comprehensive source-backed L3 timing, keep the work holistic, and stop only after the prescribed closure audit - not by silently treating a partial opcode batch as a complete machine. |
| Similar-Issue Sweep | Sweep all primary/secondary dispatch, profile metadata, prefix/LOCK outcome routes, cost publishers, ledgers, inventory, history, Queue, and TODO references for dangling, duplicated, or incorrectly broadened timing claims. |

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
| T359 S6 | Accepted a private 80386 privileged-form classifier at the successful-retirement publisher. It allocates fixed successful rows for ARPL; selector/table/MSW forms; LAR/CLTS; CR/DR/TR; FS/GS stack operations; and LSS/LFS/LGS, while transferring 80286 Appendix-B normalization, granular LSL, all delivery/fault and physical timing to T360 or cycle-exact receivers. [Evidence](../etc/evidence/t359-s6-privileged-source-ledger.md). |
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
