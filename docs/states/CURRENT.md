# Project Status

## Current Work

## M5 T385 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner approved continuous single-person execution of the ordered M5 Queue through the DeskPro/XT L3 program, and most recently confirmed the T384 Model-40 configuration and IBM/Compaq EGA ownership split. T384 is closed; this next ordered candidate is admitted only for its CPU boundary. |
| Objective | Reconcile the selected 1986 DeskPro 386 Model 40 80386DX-16 CPU/state contract against the closed shared 80386 evidence and current source, then repair only a proven CPU/state or successful-retirement gap and record the resulting Model-40 CPU ledger. |
| Non-goals | No machine/profile runtime composition, firmware execution, external-ROM manifest, memory/ROM mapping, EGA personality, keyboard/input, FDC/storage, PIC/DMA/PIT/RTC/NMI/reset binding, board/bus timing, clock/wait-state claim, x87, VME/PVI, Windows claim, or L3 decision. |
| Reference Baseline | T384 S1/S2 Model-40 selection and transfer; the closed T340--T342 80386DX evidence; `80386-closure-map.md`; DeskPro CPU candidate and shared DeskPro closure context; Intel 80386 primary documentation and project-owned probes where a source-defined CPU distinction is observable. |
| Candidate Proposal | [DeskPro 386 Model 40 80386 CPU closure](../proposals/m5-deskpro-386-profile-cpu-closure.md). |
| Files And ABI Surface | Initial audit may change only `docs/states/CURRENT.md` and T385 evidence/history. Any later source repair requires an explicit packet revision naming its CPU owner, callers, tests and ABI surface; no product/core composition or device source is admitted by this packet. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, and `docs/etc/operations/policy/source-policy.md`; CPU changes, if justified, must preserve the shared core transaction/publication owner. No exception is requested. |
| Verification | Build a source-labelled 80386DX-16 form/state matrix from the closure map and T340--T342; sweep decode, CPU profile gates, mode/paging/privilege/delivery/retirement publishers and their callers; distinguish CPU semantic evidence from later board timing; run focused existing or new project-owned CPU tests for every repaired/claimed row plus retained regressions; run the applicable current gate before acceptance. |
| Expected Markers | `M5:T385:S1:DESKPRO-CPU-LEDGER:OK` and, if a repair is proven necessary, a source-owner-specific repair marker recorded in the revised packet and evidence. |
| Asset Needs | No firmware, guest media, third-party source, emulator code or external-ROM asset is needed for the initial CPU audit. Primary Intel/Compaq documentation is research-only and is not imported. |
| Reporting Requirements | Record the Model-40 CPU identity/clock boundary, every matrix row claimed complete or transferred, source/caller/publication findings, focused proof, retained regression result, and exact functional/timing receivers. Switch to coordinator review after one complete audit/repair P; do not treat a smoke count as CPU closure. |
| Stop Conditions | Stop and revise or transfer if a required distinction depends on DeskPro device composition, firmware execution, physical clocking, waits/arbitration, an unselected corpus, VME/PVI, x87, persistent-cache/test-register model, or a new external source import. |
| Exit Criteria | The selected 80386DX-16 CPU form/state ledger has no unclassified in-scope row; every proven source gap is repaired at its earliest CPU owner with focused proof, every non-CPU row has a named later receiver, and the evidence makes no DeskPro runnable/device/timing/L3 claim. |
| Original Owner Request | Complete the ordered M5 L3 program with single-person dual-role governance, using the selected DeskPro Model 40 configuration and shared-VADP/personality allocation without importing external ROMs into the repository. |
| Similar-Issue Sweep | Search all CPU profile gates, 80386 decode/execution state, fault/interrupt/retirement publishers and their consumers for the same model-visible semantic gap; separately search profile/product descriptors only to prove that no DeskPro device/composition work is silently pulled into this CPU task. |

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
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
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
