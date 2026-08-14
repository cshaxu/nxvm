# Project Status

## Current Work

**Active.** M5 T355 S2 establishes an explicit opt-in BYOB checkpoint harness
for the retained HDD/INT13 observation; it does not run Windows Setup.

## M5 T355 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner approved continued holistic device/L3 completion and readiness-map work on 2026-08-13. S1 accepted at `8568f9e4` / `20a52a06` allocates this harness boundary. |
| Objective | Make one retained HDD/INT13 checkpoint explicitly opt-in, BYOB-configured, reproducibly invocable, and isolated from current-gate semantics without changing guest time or importing media. |
| Non-goals | No Setup execution, Windows compatibility claim, media copy/hash/path persistence, core CPU/device/timing change, current-gate registration, generic test runner, or host-time-as-guest-time behavior. |
| Reference Baseline | `20a52a06` on `master`, accepted T355 S1 readiness ledger. |
| Candidate Proposal | [Windows 3.x readiness map](../proposals/m5-windows-3x-readiness-map.md); [S1 ledger](../etc/evidence/t355-s1-readiness-ledger.md); [source policy](../etc/operations/policy/source-policy.md). |
| Files And ABI Surface | CMake opt-in checkpoint wiring, existing owner probe only if a bounded contract defect reproduces, indexed evidence/history/CURRENT. No core/profile/provider ABI or current-gate partition change. |
| Applicable Rules | Task Reading Set; execution lifecycle; source policy; architecture/coding rules for CMake/probe change; documentation rules. The harness must preserve owner-local media configuration and host/guest boundary. |
| Verification | Sweep all Windows probe registrations and media variables; prove the selected target is absent from current-gate, rejects missing opt-in media configuration deterministically, forwards only configured BYOB input, and leaves no path/media content in Git. Run configuration, focused harness check without media execution, governance, diff check, and current gate. |
| Expected Markers | A named opt-in readiness-checkpoint target/check, unchanged current-gate partition, governance and full current gate. |
| Asset Needs | No media access for implementation verification. A future owner-supplied local HDD is required only for S3 execution. |
| Reporting Requirements | Record target, configuration variable, invocation form, absent-media outcome, current-gate exclusion, host-observation boundary, and every untouched probe. |
| Stop Conditions | Stop for owner decision if correct wiring requires reading/copying media, hardcoding a local path/hash, altering current-gate, changing guest timing/device state, or creating a generic host runner. |
| Exit Criteria | One bounded opt-in checkpoint can be invoked only with explicitly supplied local media, is mechanically outside current-gate, and creates no media/provenance or guest-time side effect. |
| Original Owner Request | Fully and stably implement high-value devices and selected L3 before using a Windows consumer to decide subsequent work. |
| Similar-Issue Sweep | Include all Windows probe targets, all CMake media-target lists, fixture documentation, source policy, host-thread/sleep calls, and current-gate partition verification. |

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
| T355 S1 | Accepted a source/provenance readiness ledger: every retained Windows probe is non-current and host-observation-bound; T347/T354 storage is a regression prerequisite, not a missing feature; S2 receives a reproducible checkpoint/provenance harness before any guest run. [Evidence](../etc/evidence/t355-s1-readiness-ledger.md). |
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
