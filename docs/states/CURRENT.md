# Project Status

## Current Work

M5 T351 S3 - PC/AT keyboard device protocol and IRQ1 lifecycle
(Single-Session Mode).

## M5 T351 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 completion on 2026-08-13; T351 S2 accepted at `90d73bae`. |
| Objective | Reconcile the selected AT keyboard command and scan-code state machine: ACK/RESEND/BAT/identify, LED/typematic/scan-set parameter states, default/disable/enable/reset transitions, keyboard-owned response history, queued scan-code/typematic publication, delayed response, IRQ1 eligibility, and reset/finalize lifecycle. |
| Non-goals | No controller command/status/output-port change except necessary single-FIFO integration, no AUX-device protocol, host passthrough, firmware/physical serial timing, arbitrary scan sets, guessed keyboard error codes, port-61/PPI, generic PIC/timeline refactor, x87, or Windows claim. |
| Reference Baseline | `90d73bae` / T351 S2 accepted; [S1 ledger](../etc/evidence/t351-s1-kbc-aux-ledger.md), [S2 controller evidence](../etc/evidence/t351-s2-kbc-controller.md), IBM PC/AT Technical Reference keyboard chapter, and selected project keyboard/DOS probes. |
| Candidate Proposal | [PC/AT 8042 and AUX completeness](../proposals/m5-kbc-aux-completeness.md). |
| Files And ABI Surface | Expected: private KBC keyboard state/helpers, owner KBC-controller or dedicated keyboard smoke, CMake only if a new owner target is justified, S3 evidence, Current, and T351 history. No public host/input/profile API, second FIFO, controller/AUX protocol, PIC, or timeline ownership change. |
| Applicable Rules | Task Reading Set; one KBC FIFO and origin owner; keyboard response history cannot use controller/AUX data; validation/state transition before response publication; copied host boundary; deterministic timeline owner; primary-manual/project-probe discipline; no source import. |
| Verification | Prove full selected command matrix: `FF`, `FE`, `FD`--`F7`, `ED`, `EE`, `F0`, `F2`, `F3`, `F4`, `F5`, `F6`, invalid commands and parameter routes. Prove keyboard-only RESEND across intervening controller/AUX bytes, disable/default/enable/reset state, queued scan/typematic cancellation, delayed response, IRQ1 head lifecycle, full FIFO handling, reset/finalize, retained mapper/ingress/DOS behavior, and full current gate. |
| Expected Markers | Retain controller, keyboard mapper/host-ingress/DOS, AUX guest, mouse and timeline markers; add a dedicated S3 marker only if existing controller ownership cannot express the complete keyboard matrix. |
| Asset Needs | Published IBM keyboard/PC/AT documentation and project-owned probes only; no firmware, host capture, guest media addition, or third-party source. |
| Reporting Requirements | Record every selected command and parameter transition, response-history ownership, response/scan publication ordering, IRQ1 and reset/finalize result, reproduced defects, similar-issue sweep, and precise retained S4 boundaries. |
| Stop Conditions | Stop for a required physical serial/ACK acceptance timing model, unselected scan-set/device behavior, public/profile/host ABI change, controller/AUX semantic change, PIC/timeline mechanism defect, source import, or non-keyboard owner. Transfer rather than infer. |
| Exit Criteria | Every selected keyboard command, parameter state, response/scan byte, IRQ1 route, delay, full FIFO, reset/finalize, and copied ingress boundary has one exact owner and proof. No controller or AUX byte can alter keyboard RESEND semantics; S4 AUX device rows remain explicit. |
| Original Owner Request | Make core-machine devices stable, comprehensive, and reliable at deterministic L3 before deciding any Windows execution route. |
| Similar-Issue Sweep | Inspect all keyboard command cases, pending-write states, response/FIFO enqueue and dequeue helpers, last-output fields, scan-code and typematic writers, IRQ1 refresh paths, reset/finalize, session/mapper ingress, keyboard/AUX smokes, and profile timing binds. |

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
| T351 S2 | Accepted the selected IBM 8042 controller lifecycle: `AAh` now inhibits keyboard input, refreshes FIFO-head IRQ eligibility, returns controller `55h`, and `AEh` restores keyboard/IRQ1 publication. Command, FIFO, A20/reset, IRQ, reset/finalize, and L3 boundaries remain singly owned; keyboard/AUX device protocol stays S3/S4. 228 current-gate tests passed. [Evidence](../etc/evidence/t351-s2-kbc-controller.md). |
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
