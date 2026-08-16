# Project Status

## Current Work

**T386 open; S1--S15 accepted; S16 active.** S16 reconciles the source-sensitive
D4 memory/ROM decode before implementing any further firmware-visible Model-40
function. It may change VM Model-40 composition and use an existing generic
Core mapping capability; any new generic Core behavior requires a packet
revision and evidence.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S16. |
| Admission And Approval | Owner approved continuing T386 functional closure under the capability-ledger-first plan and explicitly requires profile behavior in VM versus generic hardware capabilities in Core. S15 selected this bounded first functional receiver; no exception. |
| Objective | Reconcile Compaq primary D4 ROM decode, E0000h RAM-replacement behavior, selected Rev-E two-chip carrier and 80386 reset alias; implement only the resulting source-supported Model-40 mapping and prove it with project-owned synthetic bytes. |
| Non-goals | No public ROM loader/YAML/catalog entry, no real ROM/media/path/hash/manifest, no BIOS execution claim, no ROM shadowing or relocation state machine, no generic PC/AT mapping change, no board timing, no physical decode-waveform claim and no L3 claim. |
| Reference Baseline | T384 S1 D4 BOM/memory ledger, T386 S14 private Rev-E carrier evidence, T386 S15 functional matrix at `6c50a9c4`, original Compaq D3PE/D4-SROMA primary material, and the DeskPro functional proposal. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md` with `docs/proposals/m5-80386-deskpro-386-l3-baseline.md`. |
| Files And ABI Surface | Model-40 addresses, register semantics, replacement selection and backing ownership remain in `src/vm/profile/model40/` and private Model-40 composition. S16 may add the smallest Core-owned generic, profile-consumable memory-device registration capability if the existing immutable-only surface is insufficient; it must contain no Compaq constants, policy or profile state. Focused synthetic mapping tests cover both boundaries. |
| Applicable Rules | `docs/rules/EXECUTION.md`, architecture/coding rules, source policy, T384/S14/S15 evidence and VM-to-Core dependency direction. VM owns Model-40 chip organization, addresses and state; Core owns generic checked memory-device registration and dispatch without profile semantics. |
| Verification | Inspect exact primary decode material transiently; record a source-labelled mapping decision or explicit configuration split; add focused low/top/replacement/immutability/reset proof; retain Model-40 S7--S15 controls; run serial current-gate, required ownership/composition/documentation gates, rebuild T386 artifact if runnable source changes, and coordinator actual-change review. |
| Expected Markers | `M5:T386:S16:D4-ROM-MAP:OK`, `M5:T386:S16:D4-REPLACEMENT:OK`, and `M5:T386:S16:D4-RESET-ALIAS:OK`. |
| Asset Needs | Primary board material and owner-managed Rev-E pair may be inspected transiently outside Git. Committed tests use only synthetic bytes; no vendor path, byte, hash, image, source text or catalogue enters the repository. |
| Reporting Requirements | Report source decision before implementation, record every map/alias and excluded configuration in indexed evidence, push one complete implementation P with artifact/gates, then coordinator actual-change review and governance P. |
| Stop Conditions | Stop and transfer if primary material cannot determine whether the Rev-E pair and 128 KiB board window are the same configuration, or if implementation requires ROM shadowing/relocation beyond the primary-supported replacement contract. A Core change must remain a generic registration/dispatch capability with no Compaq addresses, register bits, backing or policy; otherwise stop. Do not guess an overlay from an emulator or clone. |
| Exit Criteria | The selected configuration has one primary-supported immutable mapping/replacement contract; every synthetic low/top/replacement/reset access has expected read/write behavior; other configuration remains explicit and unimplemented; no protected asset is tracked; all required controls/gates and coordinator review pass. |
| Original Owner Request | Achieve complete DeskPro 386 Model 40 functionality and L3 timing under the approved queue, with a capability-ledger-first execution order and VM-owned profile details versus Core-owned generic hardware capabilities. |
| Similar-Issue Sweep | Sweep all tracked Model-40 ROM constants/providers/tests, D4 memory-map references, generic immutable mapping call sites and historical conflicting T384/S7/S14 wording. Classify each as selected mapping, distinct configuration, or historical evidence; correct current source/evidence atomically and transfer any unresolved physical/shadowing behavior. |
## Current Technical Baseline

- **Current developer artifact:** T386 S16 `vm-0-5-0387` /
  `build/output/nxvm_0_5_0387.exe`, SHA-256
  `1F294C9D0A04D5F3E32F4C06DDD6E6CC6DD543F14E5807C5BB9C9075F446213E`. Built from the current S16 worktree after its 271-test serial
  current-gate; the D4 mapping proof is retained in
  [S16 evidence](../etc/evidence/t386-s16-d4-memory-rom-map.md).
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
| T386 S15 | Progress accepted: current-source functional ledger classifies every selected Model-40 component, identifies D4 ROM/replacement mapping as the next functional receiver, and preserves physical-media, firmware and timing boundaries. Documentation governance and actual-change review pass. [Evidence](../etc/evidence/t386-s15-model40-functional-ledger.md). |
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
