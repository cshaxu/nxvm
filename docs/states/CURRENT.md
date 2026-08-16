# Project Status

## Current Work

## M5 T386 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved continuous single-person execution of the ordered M5 Queue through DeskPro/XT L3, selected the original 1986 Compaq DeskPro 386 Model 40/386-16 in T384, and approved one shared VADP core with a small Compaq personality rather than copied IBM EGA code. T386 S1 is accepted; this continuation is admitted only to define the first product-local Model-40 composition contract. |
| Objective | Specify and prove the exact VM profile-carrier/composition boundary that can represent the selected Model-40 topology without exposing a partial runnable profile: 80386DX-16/no-x87, one MiB, 128 KiB external-ROM geometry, keyboard-only/no-AUX, dual PIC/DMA, primary PIT/RTC/FDC routes, second-PIT, D4 NMI/reset/A20, Compaq fixed-disk and Compaq-EGA receiver seams. Identify which existing descriptor fields can carry facts, which falsely encode generic ATA/generated firmware/generic EGA, and the smallest source/test changes necessary before a later runnable publication. |
| Non-goals | No source implementation or runtime publication; no YAML/catalog selection; no generic-PC/AT, Model-339, ATA/HDC, IBM MFM, generated BIOS or IBM-EGA reuse as a Compaq substitute; no ROM bytes/provenance manifest; no second-PIT, NMI/reset, storage or display implementation; no board timing/L3, physical 16 MHz, x87 or AUX. |
| Reference Baseline | Accepted T386 S1 functional ledger; closed T384 selected configuration and T385 CPU ledger; current `pc_at_profile`, session, provider and machine-device source graph; architecture/coding rules; source policy; product-session/profile tests and current capability evidence. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Audit/contract-only: `docs/states/CURRENT.md`, T386 evidence/history and, only if needed, focused product-profile/test planning records. Product/core source, public interfaces, profile enum/catalog, YAML, firmware, ROM/media assets and runtime composition remain read-only. A later code S must name the exact descriptor/selection/provider surface and prove that it cannot publish incomplete hardware. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, and `docs/etc/operations/policy/source-policy.md`: one composition root and one mutable route owner; no duplicate or implicit profile truth; product-visible changes require evidence; protected firmware remains external. No exception is requested. |
| Verification | Trace every descriptor field through session selection, validation, core materialization, device configuration, firmware binding, media admission and profile/YAML tests. Produce a receiver matrix for Model-40 facts, including unavailable capability states, and prove that current runtime selection would bind generic firmware/ATA/EGA or otherwise misrepresent the Model-40. Verify a later carrier can remain private/non-runnable until all S3--S6 dependencies complete; run documentation governance and actual-change review. |
| Expected Markers | `M5:T386:S2:MODEL40-CARRIER-CONTRACT:OK` and `M5:T386:S2:NONRUNNABLE-PUBLICATION-GUARD:OK`. |
| Asset Needs | No ROM, guest media, external code or new research import is consumed. Existing T384 primary-source selection bounds the contract; unknown controller behavior remains S5 research/implementation work. |
| Reporting Requirements | Record the exact current profile field/consumer audit, truthful carrier requirements, forbidden generic substitutions, any required private availability state, source/test receivers and the condition under which a Model-40 catalog/YAML entry may first become selectable. Do not call a contract a runnable machine. |
| Stop Conditions | Stop and transfer if the carrier requires a new public ABI or profile catalog/YAML behavior, an unrecorded physical/firmware fact, ROM-byte analysis, an undocumented controller contract, a core owner change, or a new third-party import. |
| Exit Criteria | A reviewed source-level contract accounts for every selected Model-40 fact and every current false substitute, establishes a non-runnable publication guard, and gives later S3--S6 exact source/test receivers without modifying product behavior. |
| Original Owner Request | Complete the ordered M5 L3 program with full DeskPro functionality before DeskPro board timing and final audit, keeping external ROMs outside Git and retaining IBM EGA for the current-product L3 closure only. |
| Similar-Issue Sweep | Search all profile descriptors, session/profile-provider callers, firmware slots, controller selection paths and YAML-visible profile identifiers. Dispose of each hit as reusable fact, false generic substitute, Model-40 receiver or unaffected Model-339/default behavior; do not create a second composition truth. |
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
| T386 S1 | Progress accepted: the Model-40 selected-device ledger allocates every functional row to a coherent owner and transfer path, preserving the external-ROM and non-substitution boundaries before implementation begins. [Evidence](../etc/evidence/t386-s1-deskpro-functional-ledger.md). |
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
