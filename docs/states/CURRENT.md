# Project Status

## Current Work

M5 T352 S3 - selected CGA mode, aperture, and copied-frame reconciliation
(Single-Session Mode).

## M5 T352 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 implementation on 2026-08-13; T352 S1/S2 accepted at `6e68050b`/`0914e475`; S1 assigns the selected CGA digital rows to this S. |
| Objective | Reconcile the selected digital CGA text, `320x200x4`, and mode `06h` `640x200x2` state paths: mode/color and B8000 aperture interpretation, odd/even raster addressing, palette/video-enable behavior, CRTC-selected page/cursor interaction, dirty/capture publication, reset, and ROM writer use. Repair a complete shared CGA mechanism only if a focused vector reproduces a defect. |
| Non-goals | No EGA planar/controller behavior, VGA/VBE/SVGA, composite/NTSC simulation, light pen, unselected CGA geometry/modes, host renderer/pacing, firmware service expansion, generic RAM/timeline refactor, or Windows claim. |
| Reference Baseline | `0914e475`; [T352 S1 ledger](../etc/evidence/t352-s1-digital-video-ledger.md), [S2 lifecycle audit](../etc/evidence/t352-s2-vadp-register-raster.md), retained T254/T266 digital CGA records, and current CGA port/VM/DOS/ROM display tests. |
| Candidate Proposal | [PC/AT digital video completeness](../proposals/m5-vadp-digital-video-completeness.md). |
| Files And ABI Surface | Expected: private VADP CGA mechanism and focused owner smoke/evidence/history/Current only if a reproduced issue requires it. No public snapshot format, profile/host ABI, imported firmware, EGA state, renderer API, or artifact identity change. |
| Applicable Rules | One VADP owner controls selected mode, aperture, palette, raster decode, dirty/capture/reset; selected CGA variants share validation and copied publication where their memory layout agrees; CRTC state remains S2's guarded bank; VM/platform consume copied output only; deterministic VADP advance remains T346's sole timing owner. |
| Verification | Sweep mode/color ports, selected CGA snapshot decoders/palettes, B8000 reads/writes, CRTC start/cursor consumers, reset/capture dirty state, QDCGA/ROM mode writers, display copy consumers, and all CGA focused/system/DOS targets. Prove selected mode switch/reset/palette/video-enable, odd/even/address wrap, CRTC page/cursor compatibility, copied-frame nonaliasing, and unsupported mode nonpublication. Run focused and system/DOS owners, governance, diff check, and full current gate. |
| Expected Markers | Retain current CGA graphics/high-resolution port, VM CGA system/DOS, ROM-video, VADP text/status, and copied-display markers; add an owner marker only if a new mechanism fixture is necessary. |
| Asset Needs | Existing code, selected IBM CGA tables recorded in S1, and project tests only; no firmware/media addition, host capture, external source import, or composite reference dependency. |
| Reporting Requirements | Record exact selected CGA mode/register/memory/palette/raster/copy evidence, writer-reader-reset sweep, any reproduced mechanism defect and caller impact, unsupported boundary, and precise S4/S5 transfers. |
| Stop Conditions | Stop for an unselected CGA mode/register, composite/light-pen requirement, public copied-frame format or host ABI change, EGA dependency, generic memory/timeline defect, ambiguous hardware rule, or required firmware/Windows behavior outside selected paths. Transfer or seek approval rather than expand S3. |
| Exit Criteria | Every selected CGA mode/aperture/raster/palette/copy/reset row has one VADP owner and direct proof; no competing decoder, stale capture, incorrect selected address mapping, palette/video-enable publication, or unclassified firmware writer remains; all EGA and unselected CGA breadth is precisely transferred. |
| Original Owner Request | Make CPU/fpu-external core-machine devices, chips, buses, and ports comprehensive and stable at deterministic L3 before choosing a Windows execution route. |
| Similar-Issue Sweep | Sweep VADP CGA mode/color/status and snapshot helpers, CRTC text consumers, all B8000 physical accesses and observers, profile/QDCGA/ROM writers, VM copied display session/mailbox/platform readers, CGA/ROM/DOS tests, T346 timing records, Queue, and TODO transfers. |

## Current Technical Baseline

- **Current developer artifact:** T351 selects `vm-0-5-0351` /
  `build/output/nxvm_0_5_0351.exe`; its closure-build SHA-256 is
  `C20F4B631F8E63DD4D963C2F85E69D735EC05793B3DCC0C2AC11E65FA6D3FB9C`.
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
| T352 S2 | Accepted selected VADP callback/CRTC/raster lifecycle: one owner, guarded/masked selected state, status flip-flop/phase/reset/timeline proof, and no reproduced mechanism defect. [Evidence](../etc/evidence/t352-s2-vadp-register-raster.md). |
| T351 | Closed selected IBM-PC/AT 8042 controller, AT keyboard, and compatible three-byte AUX at deterministic L3: one FIFO, controller/keyboard/AUX head promotion, IRQ1/IRQ12 source lifecycle, delayed/reset/finalize, copied ingress, and exact advanced-AUX/NMI/port-61 transfers. `vm-0-5-0351` SHA-256 `C20F4B631F8E63DD4D963C2F85E69D735EC05793B3DCC0C2AC11E65FA6D3FB9C`; 228 current-gate tests passed. [History](../history/M5-T351-kbc-aux-completeness.md). |
| T350 | Closed the selected PC/AT signal graph: retained 8254/IRQ0 and MC146818/CMOS/IRQ8 owners now have focused lifecycle proof; RTC alarm and IRQF defects are repaired; speaker/PPI and parity/I/O-channel NMI remain exact corpus/contract-gated TODO transfers. 228 current-gate tests passed. [History](../history/M5-T350-pcat-platform-signals.md). |
| T349 | Closed PC/AT dual-8259A command/priority/cascade, OCW3 poll/status/special-mask/SFNM, and retained edge/level producer lifecycle; physical INTA spurious behavior transfers only to L3 bus timing. 226 current-gate tests passed. [History](../history/M5-T349-pcat-pic-compliance.md). |
| T348 | Closed PC/AT dual-8237A DMA topology, page/word layout, request/cascade/EOP, validation-before-publication, M2M lifecycle, and FDC crossing bounce path; 223 current-gate tests passed. [History](../history/M5-T348-pcat-dma-completeness.md). |
| T347 | Closed retained PC/AT FDC and ATA PIO service timing: pending command/data states, reset/control/media cancellation, one FDC-then-ATA readiness owner, trace order, focused VM/DOS/HDD evidence, and 222 current-gate tests passed. [History](../history/M5-T347-storage-controller-service-timing.md). |
| T346 | Closed deterministic PC/AT L3 convergence: one reset-safe due-event owner, equal-tick `DMA -> PIT -> PIC -> FDC -> HDC -> RTC -> KBC -> VADP` ordering, copied host boundaries, exact storage/NMI/peripheral transfers, and evidence-backed Windows handoff. [History](../history/M5-T346-core-machine-device-l3-convergence.md). |
| T345 | Closed direct-compilation strictness convergence: 251/305 direct commands are target-local strict; the 54 remaining commands have a complete 175-row ownership ledger and an exact 51-source residual production record with durable bounded admissions. No global flags, inherited-runtime rewrites, or false linked-dependency claims. [History](../history/M5-T345-direct-compilation-strictness-convergence.md). |
| T344 | Closed build-quality reproducibility: fresh configuration, 305-row truthful strict-compile matrix (130 retained strict/175 deferred), canonical 218-target current-gate registration, 53 historical fixture shapes, and strict-declaration uniqueness are mechanically verified. [History](../history/M5-T344-build-quality-reproducibility.md). |

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
