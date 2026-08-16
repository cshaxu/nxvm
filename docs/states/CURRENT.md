# Project Status

## Current Work

**T386 open; S1--S13 accepted; S14 active.** S14 corrects the private Model-40
Rev-E firmware carrier: the primary Compaq board material describes two 16 KiB
odd/even ROM chips mirrored through the standard 64 KiB system-ROM window and
its 80386 high-address counterpart. The correction stays in VM Model-40
profile/composition; core remains unchanged and continues to provide generic copied
immutable mappings and A20 behavior. It is not public BYOB/profile admission or a
Model-40 L3 claim.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S14. |
| Admission And Approval | Owner approved ongoing T386 implementation and explicitly required profile-specific behavior in `vm` and generic hardware capabilities in `core` (2026-08-16). Scope is the existing private Model-40 Rev-E carrier correction only; no exception. |
| Objective | Replace the false 128 KiB contiguous Model-40 private-ROM contract with the primary-backed two-chip odd/even 16 KiB contract, interleave it into the ROM image, mirror it in the documented low and 80386 top physical windows, and prove reset-vector execution reads the immutable image through the existing A20-wrapped top-reset alias. |
| Non-goals | No public profile/catalog/YAML/filesystem loader, no vendor-ROM/media import or commit, no runtime asset path/hash, no ROM relocation/shadow-RAM state machine, no selected-device timing or L3 claim, and no new Compaq-specific core behavior. |
| Reference Baseline | T386 S13 accepted at `2795940c`; Compaq DeskPro 386-16 Technical Spec (1986), D4-SROMA and D3PE board descriptions: standard high bank is two 16 KiB odd/even chips mirrored in the 64 KiB system-ROM space, with top-address decode. PCjs/86Box remain cross-checks only. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md` with `docs/proposals/m5-80386-deskpro-386-l3-baseline.md`. |
| Files And ABI Surface | `src/vm/profile/model40/model40.[ch]`, Model-40 private-session regressions, and CMake registration. Core remains unchanged and retains one generic A20/immutable-mapping path; VM retains the Model-40 layout and reset alias. The private C carrier contract may change atomically with all internal callers. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, source policy, T386 proposals, and existing VM/core dependency direction. The VM owns chip organization, board addresses, and the Model-40 reset alias; core owns copied read-only mappings and its unchanged generic A20 behavior. |
| Verification | New focused private Model-40 ROM-layout/reset execution smoke; retained S7--S13 Model-40 controls; serial `current-gate`; relevant VM composition/ownership/documentation verifiers; rebuild the T386 developer artifact and record its identity/hash; coordinator actual-change review. |
| Expected Markers | `M5:T386:S14:MODEL40-ROM-LAYOUT:OK` and `M5:T386:S14:MODEL40-ROM-RESET:OK`. |
| Asset Needs | Owner-managed Rev-E pair is inspected only outside Git to validate chip geometry; all committed tests use project-owned synthetic bytes. No asset path, byte, hash, manifest, or source text enters the repository. |
| Reporting Requirements | Executor confirmation, bounded progress after focused proof, complete pushed P delivery with evidence; then one-session coordinator actual-change review, acceptance/governance P, and concise handoff of relocation/shadowing/timing work. |
| Stop Conditions | Stop if primary material contradicts the two-chip/mirror address contract, if a generic Core change or shadow/relocation behavior is required, or if execution needs shadow/relocation semantics; record/transfer rather than infer behavior. |
| Exit Criteria | Invalid chip geometry is rejected; synthetic even/odd bytes are observed at both low-window halves and the top reset window; writes fault; the existing generic A20 route resolves the Model-40 top reset fetch through its profile-owned alias; a bounded 80386 reset instruction fetch consumes the correct synthetic byte; default/5170 and all retained Model-40 controls pass; no protected asset is tracked; governance and coordinator review pass. |
| Original Owner Request | Continue single-person execution under governance to complete DeskPro 386 functionality and L3 work; specifically keep profile-specific behavior in VM and generic hardware capability in core. |
| Similar-Issue Sweep | Sweep every tracked Model-40 external-ROM carrier declaration, initializer, provider call, focused test and CMake registration with `rg`; update all atomically; classify no other profile carrier as applicable; and prove the reset path rather than a direct low-memory read only. |
## Current Technical Baseline

- **Current developer artifact:** T386 S14 `vm-0-5-0386` /
  `build/output/nxvm_0_5_0386.exe`, SHA-256
  `254054B0700F9A9F286C13AD16D7214C7946C4BF4D916D75FD5257728B21E437`.
  The active S14 packet records its 269-test serial current-gate and private
  Model-40 ROM-carrier proof; durable evidence is
  [S14 evidence](../etc/evidence/t386-s14-model40-rom-layout.md).
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
| T386 S11 | Progress accepted: source-backed CECG `3C2h` bit-1 CPU-video-memory decode gate is VADP-owned; private Model-40 composition supplies only reset state. 264/264 serial current-gate and governance checks pass. Remaining `3C2h`, physical, firmware, board and L3 timing behavior remains transferred. [Evidence](../etc/evidence/t386-s11-cecg-cpu-video-gate.md). |
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
