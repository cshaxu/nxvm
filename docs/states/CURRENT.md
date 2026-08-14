# Project Status

## Current Work

**Active.** M5 T353 S3 determines whether a corpus-backed PC/AT
system-control/PPI/speaker receiver is admissible; absent that proof, it
records the exact existing transfer without adding a placeholder device.

## M5 T353 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved T353's proposed S3 sequence and its corpus-gated boundary on 2026-08-13. This S admits only the evidence decision authorized by the T353 proposal; no peripheral implementation is approved without the named corpus and hardware contract it requires. |
| Objective | Determine whether repository-held DOS or Windows corpus evidence and a documented PC/AT contract justify selecting one system-control/PPI/speaker receiver; if not, preserve an exact no-owner transfer that prevents a fabricated port-`61h` device claim. |
| Non-goals | No PPI, speaker, port `61h`, parity/I/O-channel NMI, serial, parallel, game-port, host-audio, generic port-bus, firmware, or Windows-readiness implementation; no external media acquisition or third-party-source import. |
| Reference Baseline | `de208d81` on `master`, after T353 S2's accepted exact PC/AT topology contract. |
| Candidate Proposal | [M5 PC/AT port topology and selected peripheral completion](../proposals/m5-pcat-port-topology-and-peripherals.md), S3. |
| Files And ABI Surface | `docs/states/CURRENT.md`, one indexed `docs/etc/evidence/t353-s3-*.md` record, and only a necessary exact TODO/history link. No production, CMake, test, profile, port-registration, or ABI surface may change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` S/P lifecycle and actual-change review; `docs/rules/DOCUMENT.md` authority/index/debt boundary; `docs/etc/operations/policy/source-policy.md` corpus and source boundary; T353 proposal's S3 and non-goals. |
| Verification | Search the repository's committed source, tests, probes, evidence, profile leaves, and current media classification for a named DOS/Windows port-`61h` need and for a documented 8253 channel-2/port-`61h` contract; trace actual profile and core port registrations; verify the selected result is represented by exactly one Queue/TODO receiver and no unapproved provider is added. Run documentation governance and `git diff --check`. |
| Expected Markers | `Documentation governance checks passed`; no runtime marker is applicable because no device or test is admitted. |
| Asset Needs | Repository-held sources, probes, and indexed evidence only. No guest media, firmware, local image, external source, or host-audio asset is requested or consumed. |
| Reporting Requirements | Record the corpus/contract inventory, exact port-registration result, decision, and future admission condition in indexed evidence; report a material corpus discovery before implementation. This single session must complete and push the implementation P, then independently re-read the actual change as coordinator before a governance P. |
| Stop Conditions | Stop for owner direction if a named repository corpus demonstrably requires port `61h` or a PPI/speaker device, if a redistributable hardware/manual contract requires external research, or if a truthful result needs any production/profile/port-registration change. Do not silently expand this documentation-only S. |
| Exit Criteria | Every repository-held corpus/probe and current topology source is classified; one of: (a) named corpus plus hardware contract yields a separately approved implementation S, or (b) no qualifying corpus yields an indexed exact no-owner transfer to the existing PPI/speaker TODO with its admission conditions intact. No unsupported peripheral claim or unowned port leaf remains. |
| Original Owner Request | Continue the CPU-external PC/AT device/L3 program holistically, complete selected high-value devices before Windows execution, and use evidence rather than speculative peripheral additions. |
| Similar-Issue Sweep | Classify the related port-`61h`/PPI/speaker, parity/I/O-channel NMI, serial, parallel, and game-port gaps. Confirm each has one existing bounded TODO receiver and that none is falsely implied by the default PC/AT profile or KBC envelope. |

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
| T353 S2 | Accepted S1's selected PC/AT ledger and the S2 single port-leaf/named-route contract: 75 exact directional leaves cover dual DMA, `92h`, sparse KBC/VADP/FDC, and ATA; validation precedes core publication; routing proves IRQ0/1/6/8/12/14 and FDC DMA2 against actual bindings. 230 current-gate tests passed. [S1 evidence](../etc/evidence/t353-s1-pcat-port-topology-ledger.md), [S2 evidence](../etc/evidence/t353-s2-profile-topology-contract.md). |
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
