# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | `M5 T386 S11` -- Continuation. |
| Admission And Approval | Owner-approved continuing DeskPro 386 functional and L3 programme, including the explicit core-generic / VM-profile boundary. S11 follows accepted S10 and adopts the next primary-determined CECG functional gap. |
| Objective | Implement the source-backed `3C2h` Miscellaneous Output CPU-video-memory decode gate for selected CECG, using VADP as the shared hardware owner and Model-40 composition only for reset declaration. |
| Non-goals | No other `3C2h` effects, I/O-base switching, Input Status 0, feature/special-interface hardware, live-video multiplexing, monitor/physical output, firmware/ROM import, board/raster timing, profile publication, IBM/generic EGA mutation, Windows or L3 claim. |
| Reference Baseline | `a8270369` (T386 S10 accepted); Compaq *Enhanced Color Graphics Board / Color Monitor Technical Reference Guide* (December 1986), sections 4.1--4.2; T386 S6/S9/S10 evidence; `TODO.md` CECG boundary; T386 functional and board proposals. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Evidence/owned tests, existing shared VADP state, `3C2h` port and EGA memory-provider query owner, display declaration only for reset state, and private Model-40 composition only for selected declaration. No public VM product interface, catalog or assets. |
| Applicable Rules | `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, and `docs/etc/operations/policy/source-policy.md`; VADP owns the generic device decode rule, Model-40 composition only selects its reset state. |
| Verification | Build a source-to-observable table for Miscellaneous Output bit 1; add focused VADP and selected Model-40 controls covering enabled/disabled aperture route, declared reset restoration and generic-EGA isolation; run serial current gate and applicable governance checks. |
| Expected Markers | `M5:T386:S11:CECG-CPU-VIDEO-GATE:OK`; any implementation marker must name its actual owner. |
| Asset Needs | Transient primary documentation/research and project-owned synthetic fixtures only; no third-party source, firmware, guest media, local path or binary is copied, hashed or committed. |
| Reporting Requirements | Report source-determined register semantics, implementation/evidence outcomes and every transfer; complete and push one P only after self-review, then conduct coordinator actual-change review. |
| Stop Conditions | Stop and transfer an observable lacking a primary contract, requiring alternate I/O routing, board memory availability, firmware, physical monitor behavior or board timing; do not infer from IBM EGA or copy emulator behavior. |
| Exit Criteria | The CECG-only `3C2h` bit changes whether VADP claims its EGA aperture; reset restores the declared state; generic EGA remains isolated; focused and required gates pass; all excluded behavior is explicitly transferred without a board-timing claim. |
| Original Owner Request | Continue DeskPro 386 complete functional and L3 work under the stated boundary: profile/composition in `vm`; generic hardware capability in `core`. |
| Similar-Issue Sweep | Sweep `3C2h`, miscellaneous output, EGA device-provider query/read/write routes, CECG declarations, generic EGA paths, Model-40 composition, reset paths, tests, CMake and all CECG TODO/proposal/evidence records. |
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
| T386 S10 | Progress accepted: source-backed CECG Feature Control to Environment state is VADP-owned; private Model-40 composition supplies only reset state. 262/262 serial current-gate and governance checks pass. Physical, firmware, board and L3 timing remain transferred. [Evidence](../etc/evidence/t386-s10-cecg-feature-environment.md). |
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
