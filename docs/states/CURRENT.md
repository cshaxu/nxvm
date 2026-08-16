# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation -- M5 T386 S24; T386 remains open after accepted S23 and Td S102 receiver order. |
| Admission And Approval | Owner approved on 2026-08-16 to continue original DeskPro 386 development after rapidly closing the preceding S. Scope is the next selected 765A/1.2-MB logical functional receiver only; no architecture exception or timing/L3 claim. |
| Objective | Close the source-backed Model-40 selected 765A plus one 1.2-MB raw-IMG drive logical command/state/error/recovery, DMA2/IRQ6/reset and BIOS-visible consumer gaps at the earliest reusable owner, then prove the fixed Model-40 binding. |
| Non-goals | No flux/CRC/index/rotation/physical-CHRN image semantics, board waits/service durations/arbitration, firmware or guest-media import, fixed-disk/CECG work, generic YAML variants, device-timing or L3 claim. Do not add test-only production APIs. |
| Reference Baseline | Accepted T386 S23 `f43477c1` / `efdbaf46`; artifact `vm-0-5-0389` SHA-256 `0CB4173F06E91C3BD5F9493497F2EC5480F361A23493ED7A7326000DADFB3F20`; proposal receiver two. Primary 765A/Model-40 facts and existing Core FDC/DMA/IRQ owners constrain the implementation; 86Box/MAME/PCjs may be behavior cross-checks only where primary material lacks a deterministic logical fact. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`, selected 765A/1.2-MB receiver after accepted S23. |
| Files And ABI Surface | Expected: Core FDC/media/DMA/IRQ owner changes only where shared semantics are missing; VM Model-40 profile/composition only for fixed topology/binding; focused tests/CMake/evidence/CURRENT. No public Console/YAML or test-only ABI. |
| Applicable Rules | Task Reading Set; EXECUTION lifecycle/mechanism-defect/closure rules; ARCHITECTURE shared-owner/opaque-boundary invariants; CODING C11/cohesion/test-boundary rules; source policy transient-research/no-import rule; DOCUMENT at closure. |
| Verification | P1 proves the 15-sector non-DMA port path, reset/IRQ and geometry only. P2 proves the actual DMA2 transfer. P3 adds an optional asset-free BYOB replay and records that the owner-supplied external ROM reaches a shared CPU `#UD` loop before the selected-storage consumer; the next receiver must repair that reusable startup behavior, then this S24 replay must prove the ROM/BIOS-visible consumer before final gates and acceptance. |
| Expected Markers | `M5:T386:S24:FDC-12MB-LOGICAL:OK`; `M5:T386:S24:FDC-DMA2-IRQ6:OK`; `M5:T386:S24:MODEL40-FDC-BINDING:OK`. |
| Asset Needs | Transient documentation/research only. No firmware, guest-media, local asset path, vendor hash, source text, derived binary or third-party code enters Git. |
| Reporting Requirements | Report the source-backed selected 1.2-MB contract, Core/VM owner decision, focused/current gate result, artifact identity, and physical-media/timing plus HDC/CECG transfer. Coordinator independently reviews all P changes before acceptance. |
| Stop Conditions | Stop for a missing selected-drive/controller fact, a required physical-media/timing inference, conflict with Core FDC ownership, or need for a test-only product operation. Return unresolved fact to bounded research/owner decision. |
| Exit Criteria | One shared FDC/media/DMA/IRQ/reset mechanism owns all admitted logical behavior; Model-40 has the fixed selected binding; regressions cover real DMA transfer, normal/error/reset/isolation and ROM/BIOS-visible consumer effect; no timing/physical-media/firmware/L3 overclaim; required gates pass and evidence transfers remaining work. |
| Original Owner Request | Owner-approved request, 2026-08-16: quickly close current S, perform small necessary proposal/Queue/architecture governance, then continue original DeskPro development. |
| Similar-Issue Sweep | Sweep all Core FDC/media/DMA2/IRQ6/reset callers, 1.2-MB geometry/topology profiles, Model-40 and IBM/default bindings, existing BIOS-visible storage tests, and T386 functional/timing proposal receivers. |
## Current Technical Baseline

- **Current developer artifact:** T386 S23 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `0CB4173F06E91C3BD5F9493497F2EC5480F361A23493ED7A7326000DADFB3F20`.
  Rebuilt from accepted S23 source after its 277-test serial current-gate; the
  D4-SKEY/reset proof is retained in
  [S23 evidence](../etc/evidence/t386-s23-d4-skey-reset-arbitration.md).
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
| T386 S23 | Accepted: D4-SKEY uses the existing 8042 `D1h` output-port A20/reset owner, while a failed CPU double fault produces shutdown that only a D4-configured board consumes through the existing cold-reset lifecycle. Non-D4 terminal behavior remains isolated; FDC/HDC/CECG receivers and timing remain open. [Evidence](../etc/evidence/t386-s23-d4-skey-reset-arbitration.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
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
