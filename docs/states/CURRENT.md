# Project Status

## Current Work

M5 T352 S2 - selected VADP register, CRTC, raster, and retrace lifecycle
reconciliation (Single-Session Mode).

## M5 T352 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continuous holistic device/L3 implementation on 2026-08-13; T352 S1 accepted at `6e68050b`, whose ledger assigns this exact selected register/raster mechanism. |
| Objective | Reconcile selected CGA/EGA-visible VADP CRTC/register, status/retrace, reset/finalize, and deterministic-raster lifecycle: one state owner, every selected writer/reader, masks/index/flip-flop semantics, phase/reset behavior, and source-to-snapshot publication boundary. Repair only a reproduced mechanism defect. |
| Non-goals | No selected CGA/EGA mode/aperture/pixel implementation, VGA/VBE/SVGA, composite/light-pen, firmware feature, host cadence/renderer, generic timeline refactor, Windows runtime, or unselected controller register semantics. |
| Reference Baseline | `6e68050b`; [T352 S1 ledger](../etc/evidence/t352-s1-digital-video-ledger.md); T314 CRTC bound closure; T346 S5 deterministic VADP timeline evidence; existing VADP text/status, EGA controller/CRTC, and ROM/DOS display tests. |
| Candidate Proposal | [PC/AT digital video completeness](../proposals/m5-vadp-digital-video-completeness.md). |
| Files And ABI Surface | Expected: `vadp.c`/private VADP state only if a reproduced defect requires it; focused existing/new owner smoke, evidence, Current/history. No public display snapshot, profile/host, firmware, CMake topology, or artifact version change unless a focused test needs existing-target registration. |
| Applicable Rules | VADP owns mutable display register/raster state; CRTC array index predicate and constant bound assertions remain mandatory; status reads reset only the selected attribute flip-flop; timeline owns VADP advance order; copied snapshots cannot let consumers mutate VADP; variants share one validation/publication path unless their selected hardware layout differs. |
| Verification | Audit all VADP CRTC/status/mode/color/sequencer/graphics/attribute callbacks and masks, CRTC consumers, reset/configuration/finalize, timeline advance, and snapshot dirty/publication readers. Add or strengthen focused proof for selected index/mask/readback, attribute-status flip-flop, display-enable/retrace phase and reset, same-tick deterministic advance, dirty/copy isolation, and failure/nonpublication where applicable. Run focused owners, governance, diff check, and full current gate. |
| Expected Markers | Retain `M5:T314:S2:EGA-CRTC-BOUNDARY:OK`, VADP text/status, EGA controller/port, and `M5:T346:S5:INPUT-DISPLAY-TIMELINE:OK`; add a new S2 marker only if no existing owner can express the full cross-register lifecycle. |
| Asset Needs | Existing sources/evidence and IBM CGA/EGA technical-reference tables recorded in S1; no imported code, firmware, guest media, host capture, or renderer asset. |
| Reporting Requirements | Record callback/consumer/reset/timeline/snapshot inventory, source-to-proof matrix, reproduced defect and caller sweep if any, exact selected/unselected register boundary, validation, and transfers to S3--S5. |
| Stop Conditions | Stop for a required new profile/host/firmware ABI, an unselected register/mode dependency, a generic memory/timeline defect, an ambiguous IBM register semantic, a copied-frame format change, or a need to implement mode/aperture behavior assigned to S3/S4. Transfer or seek owner approval rather than broaden S2. |
| Exit Criteria | Every selected register/CRTC/raster/retrace/reset/timeline row has one truthful VADP owner and focused proof; no stale dual writer, unsafe CRTC indexing, unmasked selected write, incorrect status flip-flop/phase/reset behavior, or consumer mutation remains; all other mode/aperture work is explicitly retained for S3/S4/S5. |
| Original Owner Request | Make CPU/fpu-external core-machine devices, chips, buses, and ports comprehensive and stable at deterministic L3 before choosing a Windows execution route. |
| Similar-Issue Sweep | Sweep all VADP callback functions and masks, `crtc[]` readers, initialization/configuration/reset/finalize, machine timeline callback and reset scheduling, snapshot/dirtiness/copy consumers, profile configuration, QDCGA/ROM writers, and VADP/CGA/EGA/timeline tests. |

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
| T352 S1 | Accepted the selected digital CGA/EGA VADP state ledger: one owner map for registers, apertures, raster, timeline, copied frame, consumers, lifecycle, and every S2--S5/TODO transfer. [Evidence](../etc/evidence/t352-s1-digital-video-ledger.md). |
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
