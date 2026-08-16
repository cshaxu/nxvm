# Project Status

## Current Work

## M5 T386 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved continuous single-person execution of the ordered M5 Queue through Model-40 functional closure. T386 S1/S2 are accepted and identify the second 8254 at `48h-4Bh` as the first blocked selected-device owner. This continuation is admitted for that shared PIT capability and Model-40-private binding only. |
| Objective | Implement one additional independently resettable/advanced 8254 instance in the shared machine owner, with an explicit configured I/O-port topology, no implicit IRQ/NMI consumer and no duplicate PIT implementation; establish the private Model-40 receiver for the documented `48h-4Bh` block. Prove primary and additional PIT state, ports and reset do not alias. |
| Non-goals | No public Model-40 profile/YAML/catalog publication, firmware/ROM, 16 MHz board timing, service placement, failsafe NMI delivery, port-`61h` semantics, speaker/audio, generic-PC/AT or Model-339 topology change, storage, EGA, ATA/IBM-MFM substitution or L3 claim. |
| Reference Baseline | Accepted T386 S1 functional ledger and S2 carrier contract; `core_machine_pit` and machine lifecycle/current time owner; T384 selected dual-8254 fact; architecture/coding rules; existing PIT reset/IRQ and current-gate tests. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md). |
| Files And ABI Surface | Expected shared-owner scope: `src/core/machine/pit.*`, `machine.*`/machine interface only if a bounded configuration operation is necessary, and focused core/VM product-profile tests; product composition may carry a private Model-40 receiver only. No public session/YAML ABI unless independently justified in the review. |
| Applicable Rules | Architecture: one state/route owner, neutral core dependency flow and product-root composition; coding: C11, cohesive owner-local helpers and no duplicate PIT route; execution/document/source policy rules apply. No exception is requested. |
| Verification | Add focused regression for configured additional PIT ports, independent count/output/reset and no primary-port alias; retain existing PIT and current-gate tests. Sweep every PIT initialize/advance/reset/finalize caller and every port-registration conflict path. Prove an unbound additional PIT cannot assert IRQ/NMI and that Model-339/default composition remains unchanged. |
| Expected Markers | `M5:T386:S3:SECOND-PIT-OWNER:OK` and `M5:T386:S3:SECOND-PIT-ISOLATION:OK`. |
| Asset Needs | No ROM, guest media, external source/code or protected asset. T384 primary evidence supplies only the selected port/topology fact. |
| Reporting Requirements | Record configured port range, ownership/lifetime, advance/reset behavior, output-consumer rule, profile receiver and exact regression results. Transfer failsafe/NMI routing and board timing unchanged to S4/later board work. |
| Stop Conditions | Stop and transfer if source review requires physical frequency/availability, a failsafe-NMI behavior decision, public profile publication, firmware analysis, unrecorded board facts, or a second machine time owner. |
| Exit Criteria | The shared machine has one tested optional additional 8254 capability with no duplicated PIT logic, no primary alias and no implicit external signal; its Model-40 private receiver is documented, while all excluded NMI/timing/publication work remains transferred. |
| Original Owner Request | Complete full DeskPro functionality before board timing/final L3, with a shared VADP core and no generic device substitution. |
| Similar-Issue Sweep | Inspect all PIT state, port registration, time-advance, reset/finalize and output-binding owners plus all profile port routes. Fix the common owner or record an explicit receiver; do not add a Model-40-only PIT clone. |
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
| T386 S2 | Progress accepted: the source-level carrier contract proves that the current profile path would falsely bind generic ATA, firmware and EGA, so Model-40 remains private until its selected device receivers are complete. [Evidence](../etc/evidence/t386-s2-model40-carrier-contract.md). |
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
