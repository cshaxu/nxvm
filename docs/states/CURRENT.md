# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | `M5 T386 S7` -- Continuation. |
| Admission And Approval | Owner approved continued single-person implementation of T386 on 2026-08-16. S1--S6 progress is retained below. This S is expanded with owner approval on 2026-08-16 to repair the shared 8042 AUX-topology owner required by the selected keyboard-only Model 40. The expansion is limited to explicit AUX absence; no other core scope is admitted. |
| Objective | Materialize a private, immutable 1986 DeskPro 386 Model 40 composition specification and bind all accepted selected hardware owners through a non-public VM session route. The route must require an abstract, owner-supplied 128 KiB external-ROM input and reject absence or invalid geometry before any default firmware is bound. |
| Non-goals | No public `vm_session_profile_kind`, Console/YAML/catalog selection, filesystem ROM loading, ROM/media commit, default/generated BIOS reuse, ATA/IBM-MFM substitution, IBM EGA selection, generic session override, timing/L3/Windows claim, board-service/wait implementation, or unrelated core behavior. |
| Reference Baseline | `749ed997` (T386 S6 acceptance), accepted S2 carrier contract, S3 second PIT, S4 D4 platform, S5 Compaq storage and S6 Compaq EGA evidence. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md), constrained by [DeskPro closure context](../proposals/m5-80386-deskpro-386-l3-baseline.md). |
| Files And ABI Surface | New private `src/vm/profile/model40/` and private session composition receiver as needed; minimal `src/core/machine/{machine,kbc}.*` AUX-topology owner/configuration, CMake and owned tests. Do not modify public Console/catalog/session-selection interfaces or firmware/media assets. |
| Applicable Rules | README Task Reading Set; EXECUTION lifecycle, P/closure, similar-issue and artifact rules; ARCHITECTURE sole owner/composition/private interface rules; CODING C11/cohesion/test rules; source policy external-ROM/BYOB boundary; DESIGN architecture/coding. Evidence records provenance and all transfers. |
| Verification | Focused private Model-40 composition smoke proves immutable selected facts, absence/invalid-ROM rejection before default-firmware bind, successful product-local binding with project-owned synthetic bytes, selected hardware observations, no-AUX command/IRQ12/host-ingress proof, and default/Model-339 negative controls. Run required configured current gate serially, applicable specialized gates, documentation governance, source/diff review and static sweep. |
| Expected Markers | `M5:T386:S7:MODEL40-PRIVATE-COMPOSITION:OK`; `M5:T386:S7:EXTERNAL-ROM-GUARD:OK`; focused smoke and evidence markers. |
| Asset Needs | No external binary is read, copied, hashed, committed or discovered. Focused proof may use only project-owned synthetic bytes; real owner-managed ROM manifest/path loading is later separately admitted. |
| Reporting Requirements | Report contract confirmation, after private carrier/composition binding, after focused proof, and final implementation P with source/asset boundary and transfers. |
| Stop Conditions | Stop for any need to publish selection, use generated/default firmware, read an owner ROM/media, modify core outside the approved AUX topology owner, select unproven FDC/CECG firmware behavior, or alter functional/timing scope; transfer unresolved item through TODO/next S. |
| Exit Criteria | A private carrier validates exactly selected Model-40 hardware facts; no default descriptor is a Model-40 truth source; all selected S3--S6 receiver configurations and keyboard-only AUX absence are materialized through production composition; ROM absence/invalid geometry cannot bind any firmware; a synthetic owned input proves generic immutable-ROM binding only; focused and required gates pass; evidence truthfully transfers firmware programming, media and timing gaps. |
| Original Owner Request | Continue implementing the queued DeskPro 386 functional and L3 programme under governance, with owner-managed ROMs outside Git and no premature public profile. |
| Similar-Issue Sweep | Sweep all `vm_session_profile_kind`, profile descriptor/validation, firmware binding, display/HDC/FDC/D4/second-PIT composition and 8042 AUX/IRQ12 ingress, profile catalog and default firmware uses. Every hit is fixed, explicitly retained as a control, or transferred. |

**T386 open; S1--S6 accepted.** The accepted [functional ledger](../etc/evidence/t386-s1-deskpro-functional-ledger.md),
[non-runnable carrier contract](../etc/evidence/t386-s2-model40-carrier-contract.md),
[second-PIT proof](../etc/evidence/t386-s3-second-pit-owner.md),
[D4 platform-control proof](../etc/evidence/t386-s4-d4-platform-control.md),
[S5 storage-controller contract](../etc/evidence/t386-s5-deskpro-storage-controller.md), and
[S6 Compaq EGA personality evidence](../etc/evidence/t386-s6-compaq-ega-personality.md)
now provide primary-backed selected functional owners without early Model-40
publication. S6 adds one shared-VADP Compaq personality for the selected CECG
identity reads and 64-colour digital capture, preserves generic EGA as its
control, and truthfully transfers CECG firmware/physical/board behavior to
TODO and the later DeskPro board receiver. The serial 256/256 current gate,
all specialized gates, and documentation governance passed; P10/P11 are
accepted after actual-change review. S7 alone may compose the accepted owners;
no runnable Model-40 or L3 claim is made here.
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
