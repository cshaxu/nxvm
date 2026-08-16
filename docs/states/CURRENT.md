# Project Status

## Current Work

## M5 T384 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner explicitly confirmed the S1 open choices: Compaq Enhanced Color Graphics plus Color Monitor, 101-key Enhanced Keyboard, and owner-managed DeskPro 386/16 Rev E ROM. The owner also directs one shared VADP core with small IBM and Compaq personality contracts; IBM EGA remains solely in current-product L3 closure, and Compaq EGA solely in the DeskPro functional/board closure path. |
| Objective | Freeze the selected original DeskPro 386 Model 40 configuration and record the profile-local EGA ownership split needed to close the T384 audit without starting device or timing implementation. |
| Non-goals | No VADP/core edit, EGA implementation, generic EGA claim, IBM EGA repair, firmware import, ROM path/hash/catalogue, profile runtime binding, CPU repair, board timing, Windows claim, or L3 decision. |
| Reference Baseline | T384 S1 primary-source system-board and Model-40 storage ledger; the owner-confirmed choices above; the current-product device closure proposal; and the DeskPro CPU, functional and timing candidates. |
| Candidate Proposal | [DeskPro Model 40 profile and capability-gap audit](../proposals/m5-deskpro-386-model40-profile-capability-audit.md). |
| Files And ABI Surface | Audit/documentation-only: `docs/states/CURRENT.md`, T384 history, T384 S2 evidence, and the directly affected DeskPro/current-product proposals. Product/core source, ABI, ROM bytes, local asset paths, hashes and runtime configuration are read-only. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, and `docs/etc/operations/policy/source-policy.md`; no exception is requested. |
| Verification | Reconcile the three owner selections against Compaq primary documentation and S1; prove the external-ROM record omits bytes, path and catalogue; prove the EGA contract has a shared VADP-core owner plus disjoint IBM-current-product and Compaq-DeskPro receivers; run documentation governance, link validation and actual-change review. |
| Expected Markers | `M5:T384:S2:DESKPRO-MODEL40-CONFIGURATION:OK` and `M5:T384:S2:EGA-PERSONALITY-OWNERSHIP:OK`. |
| Asset Needs | The selected ROM remains an owner-managed external input. This S records only its owner-approved identity constraint and no redistributable byte, local path or repository manifest. |
| Reporting Requirements | Record the configuration selection, source disposition, VADP/personality split, precise later functional and timing receivers, and the exclusion of IBM EGA from DeskPro acceptance. Complete one audit P, then switch to coordinator review before closing T384. |
| Stop Conditions | Stop if a proposed Compaq personality contract would change shared VADP semantics without a later implementation task, if IBM EGA is used as DeskPro evidence, or if the ROM record would retain protected bytes/path/catalogue. |
| Exit Criteria | Every Model-40 configuration field is selected or excluded; storage remains Compaq-specific rather than ATA/IBM MFM; one shared VADP core and two disjoint personality/closure paths are recorded; the completed T384 audit has reviewable evidence and a clean handoff to the next queued CPU task. |
| Original Owner Request | The active owner objective directs governance-compliant, single-person dual-role execution of the Queue plan through complete DeskPro 386 functionality and L3 timing. |
| Similar-Issue Sweep | Reconcile T384, all DeskPro and current-product EGA references, generic PC/AT descriptors, current VADP ownership and firmware/ROM statements so no IBM route, generic adapter, ROM asset metadata or core edit is silently claimed by the Compaq baseline. |

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
| T384 S1 | Progress accepted: Compaq primary sources freeze the Model-40 system board and storage baseline; display/monitor, physical keyboard and ROM revision remain explicit owner choices before any runnable profile claim. [Evidence](../etc/evidence/t384-s1-deskpro-model40-profile-capability-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen 5170 Model 339 deterministic-L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |
| T379 | Closed at `160a34e1`: independent re-audit accepted the then-current frozen Model-339 deterministic L3 contract. Its result is historical and is superseded for the current graph by T383. [Audit](../etc/evidence/t379-s1-model339-final-reaudit.md). |
| T377 | Closed at `1cdd5110`: final audit rejects premature Model-339 L3 acceptance; its shared input/global-gate repair and independent re-audit successors are queued. [Audit](../etc/evidence/t377-s1-model339-final-l3-audit.md). |

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
