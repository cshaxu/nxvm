# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance -- M5 Td S102; standalone post-S22 T386 continuation planning. |
| Admission And Approval | Owner approved on 2026-08-16 to quickly close S22, perform small necessary planning/proposal/Queue/architecture governance, then continue the original DeskPro development. This Td changes only planning/authority navigation; no runtime behavior, artifact, task identifier, or architecture exception. |
| Objective | Reconcile the accepted S22 D4 parity result into the active T386 functional proposal and Queue title, state the dependency-ordered remaining functional receivers, select the next D4-SKEY/reset-arbitration receiver, and record whether existing Core/VM architecture authorities require amendment. |
| Non-goals | No source, CMake, runtime, asset, firmware, media, timing, L3, YAML-variant, Queue ordering, or numeric task allocation change. Do not preallocate T386 S identifiers or make a new capability/ROM selection. |
| Reference Baseline | Accepted T386 S22 `d565b232`; `vm-0-5-0389` SHA-256 `B7DC400F3E3A8857C4699EEB6349893F2B72C70607FC29BF2C02F6E1952B5D70`; current functional proposal, Queue and M5 Roadmap. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`; shared context `docs/proposals/m5-80386-deskpro-386-l3-baseline.md`. |
| Files And ABI Surface | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`, `docs/states/QUEUE.md`, T386 evidence/history/CURRENT, and only an architecture-authority disposition if the existing Core/VM map proves insufficient. No source or ABI surface. |
| Applicable Rules | Task Reading Set; EXECUTION governance admission/closure and no-preallocation rule; DOCUMENT authority boundaries; ROADMAP M5 functional-before-timing order; ARCHITECTURE shared Core versus VM-profile ownership. |
| Verification | Inspect S22 evidence/current source ownership, current proposal, Queue, Roadmap and Architecture. Run documentation governance, link/index and actual-change review. Confirm each remaining selected functional cluster has one earliest receiver and that the next receiver is narrower than Board/L3 work. |
| Expected Markers | `M5:TD:S102:T386-POST-S22-PLAN:OK`; `M5:TD:S102:CORE-VM-DISPOSITION:OK`. |
| Asset Needs | None. No research or local asset is read, recorded or imported. |
| Reporting Requirements | Report the remaining dependency order, selected next receiver, whether a principal architecture change was necessary, changed authorities and gate result. |
| Stop Conditions | Stop for a new hardware selection, incompatible shared-owner mapping, a proposal that requires preallocating S identifiers, or a missing source fact needed to select D4-SKEY/reset scope. Return such work to an admitted implementation/research receiver. |
| Exit Criteria | Proposal explicitly reflects S22 and names dependency-ordered remaining functional receivers; Queue remains ordered/unallocated and links the clarified proposal; architecture disposition is explicit without duplicate authority; required documentation checks and coordinator review pass. |
| Original Owner Request | Owner-approved request, 2026-08-16: quickly close the current S, allow small planning/proposal/Queue and necessary architecture governance, then continue original DeskPro development. |
| Similar-Issue Sweep | Sweep T386 S21/S22 matrix/history/evidence, DeskPro closure context, Queue title/order, M5 Roadmap functional-before-timing rule, current-product variant handoff and Core/VM architecture map. |

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
