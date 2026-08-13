# Project Status

## Current Work

M5 T351 S2 - PC/AT 8042 controller mechanics and lifecycle
(Single-Session Mode).

## M5 T351 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 completion on 2026-08-13; T351 S1 accepted at `4a5d3cb1`. |
| Objective | Reconcile the selected IBM PC/AT 8042 controller state machine: command/status, one guest FIFO and delayed controller replies, command byte/interface gates, self/interface tests, D0/D1 output port A20/reset, IRQ1/IRQ12 source lifecycle, reset, finalize, and the retained deterministic readiness boundary. |
| Non-goals | No keyboard-device protocol expansion, AUX-device protocol expansion, host passthrough, second FIFO, firmware rewrite, guessed parity/timeout bits or processing latency, port-61/PPI, generic PIC/timeline refactor, x87, or Windows claim. |
| Reference Baseline | `4a5d3cb1` / T351 S1 accepted; [S1 ledger](../etc/evidence/t351-s1-kbc-aux-ledger.md), [KBC/AUX proposal](../proposals/m5-kbc-aux-completeness.md), IBM PC/AT Technical Reference (March 1984 and September 1985), and Intel UPI-41/42 manual. |
| Candidate Proposal | [PC/AT 8042 and AUX completeness](../proposals/m5-kbc-aux-completeness.md). |
| Files And ABI Surface | Expected: `kbc.c`/private KBC state only if a reproduced controller defect requires it; owner KBC-controller smoke, CMake registration only if a new owner target is necessary, S2 evidence, Current, and T351 history. No public interface, profile, host-input ABI, PIC, timeline, keyboard-device, or AUX-device change. |
| Applicable Rules | Task Reading Set; one KBC owner for FIFO and IRQ sources; variants share response-publication and reset/finalize ownership; copied host boundary; deterministic `machine.c` readiness owner; primary-manual/probe discipline; no external-source import. |
| Verification | Build a command/status/FIFO/IRQ/reset matrix for every selected controller command. Prove `AA` self-test keyboard-interface consequence and `AE` recovery, `AD`/`AE` and `A7`/`A8` head-source deassert/reassert, `20`/`60`, `AB`/`A9`, `D0`/`D1`, `D4` selection, delayed reply behind full FIFO, A20/reset callback, reset/finalize, and no stale IRQ. Re-run retained KBC/AUX/host/timeline tests and full current gate. |
| Expected Markers | Retain `M5:T227:S3:KBC-CONTROLLER:OK`, AUX/guest keyboard/mouse, ingress, and T346 timeline markers; add a dedicated S2 marker only if a new owner smoke is needed. |
| Asset Needs | IBM and Intel published documentation plus project-owned probes only; no firmware, guest media, host capture, or third-party source. |
| Reporting Requirements | Record selected-command status/FIFO/IRQ/reset outcomes, self-test source basis, any corrected defect and similar-issue sweep, command classes not selected, and retained S3/S4 device boundaries. |
| Stop Conditions | Stop for a required undocumented command/status behavior, a public/profile/host ABI change, controller processing-delay design, keyboard/AUX device semantic change, PIC/timeline mechanism defect, source import, or non-8042 owner. Transfer rather than infer. |
| Exit Criteria | Every selected controller route has exact source/proof and one FIFO/IRQ/reset owner; no self-test, command-byte, output-port, delayed-response, disabled-interface, reset, or finalize state can leave an unclassified or stale guest-visible result. Keyboard and AUX device protocol rows remain explicitly S3/S4. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at deterministic L3 before deciding any Windows execution route. |
| Similar-Issue Sweep | Inspect all KBC controller command/data port writers and readers, FIFO/reply helpers, IRQ1/IRQ12 assertions, A20/reset callbacks, reset/finalize and timeline routes, profile binds, controller/AUX tests, and CMake registrations. |

## Current Technical Baseline

- **Current developer artifact:** T346 selects `vm-0-5-0346` /
  `build/output/nxvm_0_5_0346.exe`; its closure-build SHA-256 is
  `7715C8C290969A99CCC1137D2DEEC64421FF245A22BD35287A2AED0C75A8E260`.
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
| T351 S1 | Accepted the Intel 8042/PC/AT keyboard and selected AUX manual-to-source-to-proof state ledger. It assigns controller, keyboard, AUX, FIFO, IRQ, A20/reset, profile, reset/finalize, and deterministic-L3 rows to S2--S5 or a precise advanced-AUX TODO; it makes no runtime claim. [Evidence](../etc/evidence/t351-s1-kbc-aux-ledger.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |
| T343 | Closed the four-profile CPU program: one final ledger reconciles the 8086, 80186, 80286, and 80386DX execution, protected-state, delivery, task/paging/debug, and CPU-side coprocessor-interface boundaries. VME/PVI, persistent cache, x87 execution, timing/device, and Windows/product work remain explicit external candidates. [Closure ledger](../etc/evidence/t343-s1-four-profile-cross-closure.md). |

## Recent Governance

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

- **M5 Td S76:** retired the closed T330 task-switch debt from TODO and the
  unqueued proposal surface; retained its proposal, debt report, history, and
  matrix as explicitly historical/non-current evidence; and corrected the
  stale T330 closure summary. Documentation governance and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S75:** reconciled Queue state with recorded closures: removed only
  the T323 protection/privilege, T325 paging, T328 legacy LOCK, and T330
  transition-unification candidates; retained the remaining candidate order.
  Documentation governance and diff check passed. Td work has no runtime or
  artifact change.
