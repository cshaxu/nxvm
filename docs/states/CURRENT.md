# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: T386 S13. |
| Admission And Approval | Owner-approved continuation of T386 selected-device functional closure; owner explicitly confirms that Model-40 profile data belongs in `vm` and reusable hardware behavior belongs in `core` (2026-08-16). No exception is requested. |
| Objective | Implement the primary-backed Compaq CECG `3C2h` bits 2--3 master-clock/SW1 selector and Input Status 0 observable for the selected Model-40 no-Special-Features, no-vertical-retrace-IRQ configuration. |
| Non-goals | No raster/master-clock timing effect, monitor blanking/selection effect, Special Features interface/pins, vertical-retrace IRQ, `3x8/3x9` routing, firmware/ROM publication, physical monitor signaling, board waits, or L3 claim. |
| Reference Baseline | `master` at T386 S12 accepted (`2e356458`); Compaq Enhanced Color Graphics Board / Color Monitor Technical Reference Guide, Dec. 1986, consulted transiently under source policy; T386 selected-device proposal and S9--S12 evidence. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`. |
| Files And ABI Surface | Core: `src/core/machine/display_interface.h`, `vadp.[ch]`, and owned machine test/CMake registration as needed. VM: private `src/vm/composition/session/model40_composition.c` and its owned session proof only. No public profile/YAML/ROM ABI. |
| Applicable Rules | Architecture: core is the shared generic CECG state/port owner; VM composition declares only Model-40 fixed switch/reset facts; no reverse dependency, duplicate VADP, or test-only production API. Coding: C11/project types, narrow cohesive helpers, bounded public declaration. Source: transient primary research only; no scan/ROM/media/source text/local path in Git. Documentation: active packet and truthful transfer/evidence only. |
| Verification | Add core and Model-40 focused controls proving all four SW1 selector reads, declared reset restoration, CECG-only registration, generic-EGA isolation, and existing S9--S12 retained behavior. Run configured focused tests, applicable strict/ownership checks, serial current gate, documentation governance, and similar-issue sweep. |
| Expected Markers | `M5:T386:S13:CECG-INPUT-STATUS-0:OK` and `M5:T386:S13:MODEL40-INPUT-STATUS-0:OK`. |
| Asset Needs | No ROM, firmware, guest media, local asset, or third-party source import. A project-owned synthetic Model-40 ROM fixture remains test-only. |
| Reporting Requirements | Report owner-boundary confirmation now; report implementation P with focused and gate evidence; coordinator independently reviews actual diff before a documentation-only acceptance P. |
| Stop Conditions | Stop for a missing or conflicting primary register contract, an effect requiring an unselected Special Features board/IRQ producer/physical monitor, any ROM-publication requirement, or a scope expansion beyond selected register observability; transfer it to TODO/board receiver. |
| Exit Criteria | Core owns mutable selector and Input Status 0 behavior; VM contains only the selected static switch/reset declaration; focused core and private-composition tests cover required observables and retained isolation; required gates pass; evidence/TODO are truthful; implementation and acceptance commits are pushed. |
| Original Owner Request | Continue implementing DeskPro 386 complete functionality and L3 timing under governance; latest clarification: profile-specific data belongs in VM and generic hardware behavior belongs in Core. |
| Similar-Issue Sweep | Sweep all CECG configuration initializers, `3C2h` registrations/read-write handlers, reset paths, Model-40 composition, generic-EGA setup, CMake/current-gate membership, S9--S12 controls, evidence and TODO. |

## Current Technical Baseline

- **Current developer artifact:** T382 S8 `vm-0-5-0382` /
  `build/output/nxvm_0_5_0382.exe`, SHA-256
  `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`.
  Its 251-test current-gate and corrective reclosure proof are retained in
  [S8 evidence](../etc/evidence/t382-s8-console-memory-roundtrip.md) and the
  [corrective audit](../etc/evidence/t382-s8-reclosure-audit.md).
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
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
| T386 S11 | Progress accepted: source-backed CECG `3C2h` bit-1 CPU-video-memory decode gate is VADP-owned; private Model-40 composition supplies only reset state. 264/264 serial current-gate and governance checks pass. Remaining `3C2h`, physical, firmware, board and L3 timing behavior remains transferred. [Evidence](../etc/evidence/t386-s11-cecg-cpu-video-gate.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen 5170 Model 339 deterministic-L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical and is superseded for the current graph by T383. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |

## Recent Governance

- **M5 Td S101 P1:** added a compact execution-entry table without creating
  new paths, and made coordinator semantic review explicit after structural
  documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339
  current-source re-audit before DeskPro, reconciled T380 S2 history, and made
  host/workspace path detection reject the external-assets form while retaining
  guest DOS-path examples.
- **M5 Td S99 P1:** compacted documentation gates, closure evidence, artifact
  identity, build hygiene, and recorder containment within Execution; design
  authorities remain unchanged.
- **M5 Td S98 P1:** made coordinator-review inputs, exit criteria, and the
  executor-report boundary explicit in Role cycle.
- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.
