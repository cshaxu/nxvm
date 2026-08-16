# Project Status

## Current Work

**T386 open; S1--S16 accepted; S17 active.** S17 receives the functional
selected Model-40 dual-8237A topology after S16: it must publish and prove the
board's two controller, word-channel and channel-4 cascade contract through
the shared Core DMA owner while retaining all profile selection in VM. DCLK
wait, arbitration, expansion-card endpoints and board timing remain excluded.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S17. |
| Admission And Approval | The owner approved continuing the DeskPro functional-before-timing program, and expressly requires profile policy in VM versus generic hardware capability in Core. S15 identifies this first post-S16 DMA receiver; no exception. |
| Objective | Close the selected Model-40 functional binding for two 8237A-5 controllers: byte channels 0--3, word channels 5--7 and the channel-4 cascade relationship. Publish only the generic Core configuration or observation capability actually needed, select it in Model-40 VM composition, and prove it through session-visible ports, reset and transfer behavior. |
| Non-goals | No DCLK service duration or one-wait timing claim, arbitration/ISA phase work, physical HRQ/HLDA/DACK waveform, new expansion-card or peripheral endpoint, firmware/media import, generic clone behavior, public test-only API or Model-40 L3 claim. |
| Reference Baseline | T384 S1 primary-backed Model-40 capability audit; T386 S1 and S15 functional ledgers; current Core DMA owner and T230/T269/T348 DMA evidence; accepted T386 S16 at c25d46e1. |
| Candidate Proposal | docs/proposals/m5-deskpro-386-model40-device-functional-closure.md with the DeskPro shared closure context. |
| Files And ABI Surface | VM Model-40 composition owns selection of its dual-DMA topology. Core may change only its generic DMA configuration, validation, checked state publication and existing port/transaction owner; it contains no Compaq constant, address, device policy or profile state. Tests use the retained native owner contract and no test-only production operation. |
| Applicable Rules | docs/rules/EXECUTION.md, docs/design/ARCHITECTURE.md, docs/design/CODING.md, docs/rules/ARCHITECTURE.md, docs/rules/CODING.md, source policy, the Model-40 functional proposal and VM-to-Core dependency direction. Core owns generic 8237A mechanism and transactions; VM owns selected board topology. |
| Verification | Sweep every DMA configuration caller and all primary/secondary/cascade port and transfer paths; add focused Model-40 session proof for controller topology, word route, cascade, FDC DMA2 retention and reset; retain Core DMA controls; run relevant composition/ownership checks, serial current gate, artifact rebuild if runnable source changes, documentation governance, diff check and coordinator actual-change review. |
| Expected Markers | M5:T386:S17:DUAL-DMA-TOPOLOGY:OK, M5:T386:S17:DMA-WORD-CASCADE:OK, and M5:T386:S17:DMA-RESET-BINDING:OK. |
| Asset Needs | No ROM, firmware, media or external source is needed for implementation. The existing primary-backed T384 source decision is the selection authority; tests use project-owned state and synthetic transfers only. |
| Reporting Requirements | Report the Core-versus-VM ownership decision before implementation; record all selected channels, cascade and intentionally unbound endpoints in indexed evidence; push one complete implementation P with proof and gates, then conduct coordinator actual-change review and governance P. |
| Stop Conditions | Stop and transfer if satisfying a selected functional requirement needs an expansion-bus endpoint contract, a third controller, DCLK delay, physical arbitration or undocumented board-specific DMA semantics. Stop if generic Core configuration would contain a Compaq name, address or policy, or VM would duplicate controller/transaction behavior. |
| Exit Criteria | The Model-40 composition declares exactly the selected dual-controller/cascade topology; Core validates and preserves generic configuration without profile policy; focused session proof covers byte/word ports, cascade routing, retained DMA2/FDC binding and reset; no protected asset is tracked; all applicable gates and coordinator review pass. |
| Original Owner Request | Achieve complete DeskPro 386 Model 40 functionality and L3 timing under the approved queue, capability-ledger-first order, and VM-owned profile details versus Core-owned generic hardware capabilities. |
| Similar-Issue Sweep | Sweep all tracked DMA wiring/configuration call sites, primary and secondary 8237A port maps, request/cascade handling, reset behavior, Model-40 composition and focused machine tests. Classify every hit as shared Core mechanism, VM topology selection, another profile consumer, or later timing/endpoint work; repair the owner or explicitly transfer the boundary. |
## Current Technical Baseline

- **Current developer artifact:** T386 S17 `vm-0-5-0388` /
  `build/output/nxvm_0_5_0388.exe`, SHA-256
  `1B870632BA6A1F3EA15296285CBF13A76CD03BC8D4E9201E4CD5D8345D93FC04`.
  Built from the S17 worktree after its 272-test serial current-gate; the
  pending dual-DMA mapping proof is retained in
  [S17 evidence](../etc/evidence/t386-s17-model40-dual-dma.md).- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
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
| T386 S16 | Accepted: primary-bounded D4 compatibility RAM, low-active replacement/write-protection control, selected diagnostic read, reset state and generic checked overlay dispatch are implemented with synthetic proof. D4 parity/firmware copy, remaining device functionality, board timing and L3 remain open. [Evidence](../etc/evidence/t386-s16-d4-memory-rom-map.md). |
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
