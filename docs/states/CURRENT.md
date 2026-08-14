# Project Status

## Current Work

**Active.** M5 T353 S4 composes the selected PC/AT topology through session
creation/reset, deterministic timeline service, and firmware-visible state
before the task-level closure audit.

## M5 T353 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T353's proposed S4 sequence and the CPU-external selected-device/L3 completion direction on 2026-08-13. This S may repair only a reproduced selected-topology composition, reset, timeline, or firmware-visible defect after a full owner/caller sweep. |
| Objective | Prove that the accepted default PC/AT directional-port and named-route contract is preserved through ordinary VM-session creation and reset, core deterministic-L3 timeline rearming, and firmware-visible reset state; close T353 only if no selected composition defect remains. |
| Non-goals | No new peripheral, PPI/speaker, NMI producer, serial/parallel/game interface, host passthrough, physical bus wait state, INTA waveform, controller-duration, cycle-exact timing, firmware feature, Windows execution claim, or generic port-bus framework. |
| Reference Baseline | `8a68e861` on `master`, after T353 S3's accepted no-owner peripheral decision. |
| Candidate Proposal | [M5 PC/AT port topology and selected peripheral completion](../proposals/m5-pcat-port-topology-and-peripherals.md), S4. |
| Files And ABI Surface | Expected: one owner-local VM/session smoke, its CMake target/current-gate registration, T353 evidence/index/history/current closure. Production changes only for a reproduced selected composition/reset/timeline/firmware defect, with all writers/readers/registration/reset/finalize consumers swept. No public ABI change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` S/P lifecycle, mechanism defect and task closure audit; `docs/rules/ARCHITECTURE.md` single ownership/composition boundary; `docs/rules/CODING.md` test boundary; `docs/rules/DOCUMENT.md` current/history/evidence topology; T353 proposal; T346 deterministic-L3 and T353 S1--S3 evidence. |
| Verification | New focused owner smoke must create an ordinary session; compare every selected profile leaf and all named route bindings to the frozen core both before and after session reset; prove sparse non-leaves remain absent; mutate reset-owned selected state then reset and prove reset/firmware restoration, reset vector, timeline `now`/pending-event rearm, and no stale topology publication. Configure, exact current-gate discovery, focused marker, documentation governance, diff check, and full current gate must pass. |
| Expected Markers | New `M5:T353:S4:PCAT-COMPOSITION:OK`; exact `current.vm-pcat-composition-s4-smoke`; `Documentation governance checks passed`; full current gate passes. |
| Asset Needs | No external asset, guest media, firmware image, or external source. The committed default profile and generated firmware path are the only inputs. |
| Reporting Requirements | Record the initial/reset topology, route, timeline, firmware-visible, and sparse-port proof; state every reset/finalize/caller sweep result and each unchanged transfer. This single session must push one complete implementation P before independently reviewing actual changes as coordinator. |
| Stop Conditions | Stop for owner direction if a proof requires an unselected peripheral, an external corpus, a physical timing model, a generic session/port abstraction, or a shared core/firmware lifecycle change whose full caller/failure sweep exceeds the selected topology boundary. |
| Exit Criteria | The focused proof covers creation and reset lifecycle of every selected leaf/route, selected mutable reset state, deterministic timeline rearm, firmware-visible reset vector, and sparse-port absence; any reproduced in-scope defect is repaired with mechanism evidence; full gates pass; all remaining optional/per-physical-timing gaps are explicitly transferred; then T353 receives a task-level closure audit. |
| Original Owner Request | Continue the CPU-external PC/AT device/L3 program holistically, reach a stable reliable selected L3 machine before deciding the next package, and avoid speculative peripherals or Windows as a shortcut. |
| Similar-Issue Sweep | Review VM session initialization, reset callback, provider lifecycle reset/finalize, profile firmware reset, core cold reset, profile leaf/route validation, FDC/HDC/CMOS/VADP/KBC/DMA/PIC/PIT bindings, and timeline rearm. Classify physical bus timing and optional peripherals only as transfers. |

## Current Technical Baseline

- **Current developer artifact:** T352 selects `vm-0-5-0352` /
  `build/output/nxvm_0_5_0352.exe`; its closure-build SHA-256 is
  `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`.
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
| T353 S3 | Accepted S1's selected PC/AT ledger, S2's exact 75-leaf/named-route contract, and S3's repository-corpus decision: no port-`61h` PPI/speaker corpus or hardware contract exists, so its exact `TODO(Medium)` receiver and the separate NMI and optional-interface receivers remain authoritative. 230 current-gate tests passed. [S1](../etc/evidence/t353-s1-pcat-port-topology-ledger.md), [S2](../etc/evidence/t353-s2-profile-topology-contract.md), [S3](../etc/evidence/t353-s3-pcat-peripheral-admission-decision.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; `vm-0-5-0352` SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |

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
