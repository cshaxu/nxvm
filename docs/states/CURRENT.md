# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation `M5 T386 S4`; T386 is the latest open numeric task and S1--S3 are accepted. |
| Admission And Approval | Owner approved continuation and the selected Compaq EGA personality on 2026-08-15. S4 covers the S1-ledgered D4 NMI/reset/A20 and port-`61h` state only; no rule exception. |
| Objective | Add one optional D4 platform-control receiver at `61h`: source-defined IOCHK enable/status and failsafe enable/status, NMI publication through the existing CMOS `70h` mask, an explicitly configured auxiliary-PIT output route, cold-reset visible state, and a KBC reset/A20 composition boundary. |
| Non-goals | No Model-40 publication, firmware/ROM, storage/FDC/EGA, speaker/audio, speed control, timing/pulse duration, generic PC/AT port-`61h` change, or inferred latch-clear/PAL behavior. |
| Reference Baseline | Accepted T386 S1--S3, T384 S1 audit, and `master` at `1d1d9185`. Primary corpus establishes D4 `61h` bits 3/6 and 2/7, CMOS NMI masking, reset paths and two 8254s. Manufacturer diagnostics corroborate cold-visible bits 0/1/3=1 and 2/7=0 plus failsafe interrupt/NMI observability. 86Box/PCjs are non-authoritative cross-checks. |
| Candidate Proposal | [DeskPro 386 Model 40 selected-device functional closure](../proposals/m5-deskpro-386-model40-device-functional-closure.md), NMI/reset ledger row. |
| Files And ABI Surface | `src/core/machine/{machine.c,machine.h,machine_interface.h}`, and KBC files only if required for an explicit callback; one `tests/machine/` smoke; `CMakeLists.txt`; S4 evidence. Existing planar-parity API/semantics remain unchanged. |
| Applicable Rules | Task Reading Set; Execution S/P lifecycle, actual-change review, evidence and similar-issue sweep; Contributing; Architecture/Coding design and rules; source policy. Invariants: optional/default-off composition, atomic port registration, reset/finalize deassertion, no test-only ingress, source-unknown transfer, and no public profile before S3--S6. |
| Verification | Focused smoke proves default isolation, D4 cold state, documented port visibility, IOCHK/failsafe NMI with mask/unmask behavior, selected auxiliary-PIT output/reset, and planar-parity non-regression. Build focused target, run CTest and current-gate; record commands/results and sweep. |
| Expected Markers | `M5:T386:S4:D4-PLATFORM-PORT:OK`; `M5:T386:S4:D4-NMI-MASK:OK`; `M5:T386:S4:D4-FAILSAFE-ROUTE:OK`; `M5:T386:S4:D4-RESET-ISOLATION:OK`. |
| Asset Needs | No asset is loaded, copied, bundled or committed; external ROM is research-only. |
| Reporting Requirements | Executor confirms, reports owner/source boundary, self-reviews and pushes one complete implementation P. Coordinator then inspects actual changes and pushes acceptance P only if exit criteria hold. |
| Stop Conditions | Stop and transfer undocumented bit ownership, latch clear, timer channel, reset duration or PAL transformation; likewise profile publication, ROM execution, storage, EGA, timing, AUX policy or generic-PC/AT changes. |
| Exit Criteria | Default-off optional owner; no port conflict; documented cold/port/NMI behavior and explicit PIT route proven; no duplicated KBC mutation; parity/default controls retained; evidence separates proof from transfer; pushed implementation and acceptance P. No runnable/L3 claim. |
| Original Owner Request | Continue selected original 1986 DeskPro 386 Model 40 functional closure: 386-16, 1 MiB, external ROM, 1.2 MB floppy/Compaq controller, Compaq EGA, no x87/AUX; 86Box/MAME/PCjs cross-check only when primary manuals are incomplete. |
| Similar-Issue Sweep | Search tracked `src`, tests, CMake and T386/T384 records for `port.?61|0x0061|planar_parity|set_nmi_mask|auxiliary_pit|kbc.*output|request_reset`; classify each production hit as retained IBM parity, shared receiver or transfer. No static forbidden-shape gate: intentional distinct owners share port number; negative controls prove isolation. |
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
