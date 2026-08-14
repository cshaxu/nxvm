# Project Status

## Current Work

**Active.** M5 T355 S4 reconciles the readiness map and closes its bounded
checkpoint/provenance task; it does not claim Windows compatibility.

## M5 T355 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continued holistic device/L3 completion and readiness-map work on 2026-08-13. Accepted S1--S3 commits through `40638d7d` authorize final reconciliation and T355 closure. |
| Objective | Reconcile the complete four-probe ledger, selected HDD/INT13 result, source/media boundary, current-gate partition, accepted T347--T354 prerequisites, and deferred device/product work; close T355 only with truthful transfers. |
| Non-goals | No Setup, installation, Standard/Enhanced Mode, new guest/media execution, media persistence, core/device/timing implementation, current-gate promotion, or Windows compatibility claim. |
| Reference Baseline | `40638d7d` on `master`, accepted T355 S3 HDD/INT13 checkpoint. |
| Candidate Proposal | [Windows 3.x readiness map](../proposals/m5-windows-3x-readiness-map.md); [S1 ledger](../etc/evidence/t355-s1-readiness-ledger.md); [S2 harness](../etc/evidence/t355-s2-opt-in-checkpoint-harness.md); [S3 checkpoint](../etc/evidence/t355-s3-hdd-int13-checkpoint.md). |
| Files And ABI Surface | Documentation state, task history/proposal retention, Queue/TODO/evidence indexes, and stale readiness wording only. No source, build, runtime, media, current-gate, or ABI change. |
| Applicable Rules | Task Reading Set; execution lifecycle and closure audit; documentation rules; source policy. Existing device/timing claims must retain their accepted owner/evidence boundaries. |
| Verification | Mechanically sweep all Windows probe targets, media variables, current-gate registration and CTest discovery; inspect T347--T354 and open TODO/Queue transfers; validate documentation governance, Queue links, and diff check. |
| Expected Markers | No `windows31` current-gate tests; one retained opt-in checkpoint target; governance and Queue-link validation; one truthful T355 closure record. |
| Asset Needs | None. No media access or checkpoint rerun occurs in S4. |
| Reporting Requirements | Record every probe disposition, approved checkpoint result, untouched host boundary, current-gate result, accepted prerequisite, unresolved device/product transfer, and exact T355 closure outcome. |
| Stop Conditions | Stop for owner decision if reconciliation reveals an unowned or contradictory core/device/timing claim that needs implementation, media access, or a new compatibility contract rather than a precise transfer. |
| Exit Criteria | T355 is closed only if its one checkpoint is fully bounded, every non-selected Windows probe and device/product dependency has an owner/transfer, proposal/history/Queue topology is valid, and no Windows compatibility or physical-L3 overclaim remains. |
| Original Owner Request | Fully and stably implement high-value devices and selected L3 before using a Windows consumer to decide subsequent work. |
| Similar-Issue Sweep | Inspect all `vm_windows31_*` targets/sources, CMake media/current lists, fixture record, T347--T354 histories/evidence, current capability baseline, Queue, TODO, and source-policy references. |

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
| T355 S3 | Accepted the single approved HDD/INT13 checkpoint: logical input identity matched the approved local-media record; BIOS geometry and MBR/VBR reads reached ATA in two commands; no Setup/boot claim, no repository media provenance, and S4 receives the full reconciliation. [Evidence](../etc/evidence/t355-s3-hdd-int13-checkpoint.md). |
| T354 | Closed selected L3 bus timing: core-local checked CPU/DMA transaction begin/commit/cancel trace, deterministic CPU-retire then DMA/PIT/PIC then FDC/HDC ordering, reset cancellation/trace continuity, and retained firmware/DOS consumers. Exact bus cycles, pin waveforms, physical wait tables, host time, and Windows readiness remain explicit transfers. 234 current-gate tests passed. [History](../history/M5-T354-physical-l3-bus-timing-convergence.md). |
| T353 | Closed selected PC/AT topology: the default profile's 75 exact directional leaves and five named routes validate before core publication, remain exact through session reset and deterministic-L3 rearm, and have firmware-visible reset proof. Optional PPI/speaker, NMI, and interface breadth remain corpus-gated TODO transfers; physical bus timing transfers to the next L3 package. 231 current-gate tests passed. [History](../history/M5-T353-pcat-port-topology-and-peripherals.md). |
| T352 | Closed selected digital CGA/EGA: one VADP owner for registers, apertures, raster, planar state, reset, and capture; deterministic VADP-after-KBC timing and copied session/mailbox composition; `vm-0-5-0352` SHA-256 `37A00874C05FB402151AF1A77BD28796D03B2A738D6F3EC08761E4541F2D6BB4`; 229 current-gate tests passed. [History](../history/M5-T352-vadp-digital-video-completeness.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |

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
