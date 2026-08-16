# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation -- M5 T386 S23; latest open T386 retains accepted S22 progress and Td S102 receiver order. |
| Admission And Approval | Owner approved on 2026-08-16 to continue the original DeskPro development after the small planning/governance work. Scope is the next source-backed D4-SKEY A20 and reset-arbitration functional receiver; no architecture exception or timing claim. |
| Objective | Define and implement the selected Model-40 D4-SKEY A20 control and the D4 shutdown/program/power plus 8042 reset-arbitration behavior, reusing Core A20/reset owners where semantics match and retaining only Model-40 board policy in VM/profile composition. |
| Non-goals | No physical reset pulse, bus-cycle/READY timing, firmware execution, guest media/ROM import, generic variant schema, D4 parity redesign, FDC/HDC/CECG work, DeskPro board timing or L3 claim. Do not add a test-only reset/A20 API or reuse IBM planar semantics. |
| Reference Baseline | Accepted Td S102 `610c5262`; T386 S22 `d565b232`; `vm-0-5-0389` SHA-256 `B7DC400F3E3A8857C4699EEB6349893F2B72C70607FC29BF2C02F6E1952B5D70`. Primary Compaq Model-40 technical material is researched transiently under source policy; existing Core A20, 8042 reset and D4 platform owners constrain the source. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`, first remaining receiver after S22. |
| Files And ABI Surface | Expected: focused Core A20/reset owner only if a reusable operation is missing; Model-40 D4/profile/composition only for board-local wiring; focused tests/CMake and T386 evidence/history/CURRENT. No Console/YAML ABI or public test operation. |
| Applicable Rules | Task Reading Set; EXECUTION lifecycle/mechanism-defect/closure rules; ARCHITECTURE shared-owner and opaque-boundary invariants; CODING C11/cohesion/test-boundary rules; source policy transient-research/no-import rule; DOCUMENT at closure. |
| Verification | Establish the primary D4-SKEY/reset facts before code. Sweep Core A20/reset, 8042 output/reset, D4 port/control paths and existing Model-40 mappings. Add focused normal/reset/interaction/isolation regressions, run strict-owner/static gates, serial current gate, documentation gate, diff check and actual-change review. |
| Expected Markers | `M5:T386:S23:D4-SKEY-A20:OK`; `M5:T386:S23:D4-RESET-ARBITRATION:OK`; `M5:T386:S23:CORE-VM-RESET-OWNER:OK`. |
| Asset Needs | Transient primary technical reference only. No firmware, guest-media, local asset path, vendor hash, source text, derived binary or third-party code enters Git. |
| Reporting Requirements | Report the source-backed D4-SKEY/reset contract, Core/VM ownership decision, focused/current gate result, artifact identity, and remaining FDC/HDC/CECG/timing transfer. Coordinator independently reviews all P changes before acceptance. |
| Stop Conditions | Stop for missing/ambiguous primary D4-SKEY or reset fact, a required physical timing inference, a need to bypass the existing 8042 reset owner, conflict with existing Model-40 D4 behavior, or a test-only production operation. Return unresolved hardware fact to a bounded research/owner decision. |
| Exit Criteria | One source-backed A20/reset arbitration contract has one shared owner per reusable mechanism and Model-40-local policy only where needed; regressions prove selected control/reset behavior and isolation; no timing/firmware/media/L3 overclaim; required gates pass and evidence transfers remaining work. |
| Original Owner Request | Owner-approved request, 2026-08-16: quickly close current S, perform small necessary proposal/Queue/architecture governance, then continue original DeskPro development. |
| Similar-Issue Sweep | Sweep all Core A20/reset/8042/keyboard reset callers, D4/SKEY/control mapping, Model-40 composition/reset, IBM planar and default-PC/AT isolation, and T386 functional/timing proposal receivers. |

## Current Technical Baseline

- **Current developer artifact:** T386 S22 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `B7DC400F3E3A8857C4699EEB6349893F2B72C70607FC29BF2C02F6E1952B5D70`.
  Rebuilt from accepted S22 source after its 276-test serial current-gate; the
  D4 parity/IOCHK proof is retained in
  [S22 evidence](../etc/evidence/t386-s22-d4-parity-diagnostic.md).
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
| T386 S22 | Accepted: D4 lane-parity diagnostics and the logical IOCHK/NMI path now consume one location-bearing Core event owner; normal covered RAM writes clear IOCHK, and IBM planar parity remains isolated. D4-SKEY/reset arbitration, other functional receivers and timing remain open. [Evidence](../etc/evidence/t386-s22-d4-parity-diagnostic.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.