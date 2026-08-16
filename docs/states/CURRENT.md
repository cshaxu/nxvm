# Project Status

## Current Work

## M5 T386 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner approved continuous single-person execution of the ordered M5 Queue through the DeskPro/XT L3 program, selected the original DeskPro 386 Model 40 configuration in T384, and required a small Compaq EGA personality around the shared VADP core rather than an IBM-derived adapter. T385 is closed; this next ordered candidate is admitted for functional-gap allocation only. |
| Objective | Build the complete selected Model-40 functional capability ledger and ownership plan for the first runnable Compaq composition: memory/ROM contract, PIC/DMA/PIT/RTC/KBC/A20/NMI/reset, selected 1.2 MB/FDC and Compaq fixed-disk subsystem, 101-key keyboard, and Compaq EGA/color personality. Classify every row as reusable, partial, missing or excluded, with its earliest repair owner and a bounded later S receiver. |
| Non-goals | No device implementation, runtime DeskPro descriptor, ROM-byte import or manifest, ATA/HDC or IBM MFM substitution, IBM EGA completion, board/bus/monitor timing, physical 16 MHz claim, x87, AUX/IRQ12, Windows claim or Model-40 L3 decision. |
| Reference Baseline | T384 S1/S2 selected Model-40 ledger and EGA ownership allocation; T385 CPU closure; the DeskPro functional candidate and shared DeskPro context; current capability baseline/T373 classification; Compaq primary documentation already recorded by T384; current source and project-owned test inventory. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Audit-only: `docs/states/CURRENT.md`, T386 evidence/history and, if needed, directly affected DeskPro proposal detail. Product/core source, public ABI, ROM/media assets and runtime composition are read-only. A later implementation S must name exact source owners, callers, tests, profile contract and ABI before code changes. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, and `docs/etc/operations/policy/source-policy.md`; one mutable state/route owner, neutral dependency flow and product-root-only composition remain required. No exception is requested. |
| Verification | Sweep all selected components through existing core and VM owners, ports/routes, reset/publication paths, tests and current capability evidence; distinguish generic product support from profile evidence; verify no Compaq descriptor/personality/controller exists; reconcile every selected row exactly once to reusable/partial/missing/excluded plus earliest owner and S receiver; run documentation governance, link validation and actual-change review. |
| Expected Markers | `M5:T386:S1:DESKPRO-FUNCTIONAL-LEDGER:OK` and `M5:T386:S1:DESKPRO-OWNER-ALLOCATION:OK`. |
| Asset Needs | No ROM, guest media, third-party source or emulator code is consumed. Existing primary documentation remains research evidence only; any later BYOB ROM binding needs a separately admitted profile-composition contract. |
| Reporting Requirements | Record every selected device/capability, current source disposition, exact earliest owner, functional S boundary, later board-timing receiver and exclusion. Explicitly report the smallest coherent implementation slices and their dependency order; do not use a generic product device as completion evidence. |
| Stop Conditions | Stop and transfer if a required functional contract depends on a ROM byte, undocumented Compaq controller behavior, physical bus/monitor timing, a missing corpus, a device outside the selected Model-40 configuration or a new third-party import. |
| Exit Criteria | The selected Model-40 functional ledger has no unclassified row, no ATA/IBM/IBM-EGA substitution, no profile-composition claim, and a reviewable dependency-ordered implementation allocation that can be admitted one coherent owner boundary at a time. |
| Original Owner Request | Complete the ordered M5 L3 program with single-person dual-role governance, including full DeskPro functionality before DeskPro board timing and final audit, while keeping external ROMs outside the repository. |
| Similar-Issue Sweep | Search all generic/default PC/AT, Model-339 and current-product device/profile bindings for accidental use as a DeskPro substitute; search every selected component's port, IRQ/DMA, reset and registration owner so a later S repairs the earliest shared owner or a clearly named Compaq-local contract, never a duplicate route. |

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
