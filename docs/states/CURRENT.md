# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation T386 S6. |
| Admission And Approval | Owner approved T386 continuation and the one-VADP-core/two-small-personality-contract direction in this conversation; 2026-08-16. Scope is core-only Compaq EGA functional personality, with no publication. |
| Objective | Add one source-backed Compaq Enhanced Color Graphics/Color Monitor personality around the shared VADP owner: selected 64-colour digital palette conversion and determinate CECG identity-status ports, while retaining generic EGA behavior as the control. |
| Non-goals | No `src/vm` change; no DeskPro profile/catalog/YAML/firmware/media publication; no copied VADP core; no IBM EGA claim; no option-ROM, light-pen, dynamic monitor/configuration, physical cable/waveform, arbitration, wait, or raster-timing claim. |
| Reference Baseline | `2062fca0` T386 S5 P9 on `master`; current artifact remains T382 S8 `nxvm_0_5_0382.exe` SHA-256 `6507E88A85F9E85ED4EB12D1B0B376DED5894771D178E3D1B2B5B89202BC8D57`. |
| Candidate Proposal | [DeskPro 386 Model 40 Selected-Device Functional Closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md); T384 S2 configuration and T386 S2 carrier evidence are binding. |
| Files And ABI Surface | `src/core/machine/{display_interface.h,vadp.h,vadp.c,machine.c}` only as required, one focused `tests/machine` smoke, `CMakeLists.txt`, `cmake/verify_t344_historical_fixture_shapes.cmake`, `cmake/verify_t345_deferred_direct_ownership.cmake`, `docs/states/CURRENT.md`, indexed S6 evidence and transfer debt. Public core config may add an immutable personality selector; no mutable layout crosses modules. |
| Applicable Rules | Architecture: one VADP mutable owner, core never depends on VM, composition only at approved root, no product-visible publication. Coding: C11/project types, bounded owner-local helpers, no test-only API or duplicate owner. Source: documentation facts only, no third-party source/firmware/media import. Documentation: active packet and indexed evidence remain authoritative. |
| Verification | Build the focused smoke and current gate serially; inspect generic EGA negative controls; reconcile the discovered three-entry T344 direct-fixture inventory drift; run specialized and documentation governance gates at closure. The focused smoke proves Compaq-only port ownership/status values, 6-bit digital palette mapping, reset persistence and generic non-regression. |
| Expected Markers | `M5:T386:S6:COMPAQ-EGA-PERSONALITY:OK`; `M5:T386:S6:COMPAQ-EGA-SWEEP:OK`. |
| Asset Needs | The Compaq December 1986 Enhanced Color Graphics Board/Color Monitor Technical Reference Guide is transient research only; no ROM, guest media, third-party source, machine-local path, or downloaded asset enters Git. |
| Reporting Requirements | Report admission confirmation, focused implementation/proof, full serial-gate outcome, source provenance boundary, transfers, pushed executor P, then coordinator actual-change review and governance closure P. Include compatible user changes after review. |
| Stop Conditions | Stop for an undocumented required behavior, a need for ROM/firmware/media import, a required VM/profile/catalog edit, a public ABI conflict, or a material expansion beyond selected functional personality; revise/admit a later receiver instead. |
| Exit Criteria | One shared VADP owner supports generic and Compaq selected behavior without copied mechanics; focused regressions prove selected Compaq state and generic control; all required gates pass; evidence maps source facts, implementation and transfers; complete P is pushed and independently reviewed. |
| Original Owner Request | Continue work after authorization; Compaq EGA must be a small personality around the shared VADP core, while IBM EGA remains current-product L3 and Compaq EGA is DeskPro-only. Owner also authorizes including their relevant changes in commits. |
| Similar-Issue Sweep | Defect class: generic EGA palette/port behavior silently standing in for a selected non-IBM EGA personality. Search tracked `src/core`, `src/vm`, tests, CMake and T384/T386 records for VADP/EGA palette/port/profile consumers. Shared VADP hits are repaired once here; VM/publication hits are unchanged and explicitly transferred to S7; physical/firmware/timing variants transfer to the DeskPro board task/TODO. |
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
| T386 S3 | Progress accepted: one optional shared 8254 now owns isolated `48h-4Bh` state, ports, reset and machine-time advance without an implicit signal consumer; failsafe/NMI and publication remain transferred. [Evidence](../etc/evidence/t386-s3-second-pit-owner.md). |
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
