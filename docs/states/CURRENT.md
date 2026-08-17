# Project Status

## Current Work

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation - T390 S3. |
| Admission And Approval | Owner approved continuation on 2026-08-17, including the necessary shared Core implementation for 80386 qualification. The owner forbids test-only APIs and requires reusable hardware mechanisms in Core; external ROM/media remains owner-managed and uncommitted. |
| Objective | Implement the minimal lifecycle-owned, opt-in Core retirement-observation provider that receives a copied pre-execution instruction point and the classifier disposition/timing result before physical-contract rejection or elapsed/device-time publication. |
| Non-goals | No external ROM/media execution or retention, raw recorder/harness, Console UX, VM/profile policy, physical-contract selection, guessed timing row, board-clock conversion, device timing, CPU semantic change, second executor, or Model-L3 claim. |
| Reference Baseline | T390 S1 classifier inventory and T390 S2 observation contract. Existing diagnostic callback owns the pre-execution copied instruction point; `core_machine_instruction_cost()` and physical rejection own classification/publication order; generic trace ABI remains unchanged. |
| Candidate Proposal | [80386 physical-retirement qualification](../proposals/m5-80386-physical-retirement-qualification.md). |
| Files And ABI Surface | Core machine observation interface/implementation and private state; `machine.c` classifier/publication junction; focused Core smoke and CMake registration; T390 evidence/history/current baseline and artifact configuration if required by runnable-path identity. No VM/profile source. |
| Applicable Rules | Architecture single-owner, opaque copied public contract, lifecycle mutation and production-path invariants; C11/type vocabulary/minimal durable interface; execution artifact, actual-change review, source policy and documentation topology. No exception is requested. |
| Verification | Focused Core smoke must prove disabled-path neutrality; stopped/paused-only install/remove; copied pre-execution bytes/PC and relevant context; exact/unallocated disposition emitted before publish; physical unallocated observation then fault with unchanged elapsed/timeline; reset/failure behavior; retained trace and current gate. Run source/caller sweep and full configured current gate; build/copy T390 artifact and record identity/hash if code completes. |
| Expected Markers | `M5:T390:S3:RETIREMENT-OBSERVATION:OK`; static single-publisher/observer-owner marker; current-gate pass. |
| Asset Needs | None for implementation proof. The S2 external corpus remains deferred and no ROM/media/local path/hash/trace may enter repository or test fixture. |
| Reporting Requirements | Report public capability/lifetime, exact publication ordering, focused and full verification, artifact identity if emitted, commit/push state, and any remaining row-identifier/corpus receiver. |
| Stop Conditions | Stop and transfer if obtaining required copied pre-execution context needs a second CPU callback path, exposing a raw executor pointer, changing generic trace ABI, a CPU semantic repair, unbounded storage/output, or an external asset. Do not substitute a classifier route label for an Intel timing row. |
| Exit Criteria | One opt-in production Core observer is lifecycle-bound and disabled by default; it observes each classified successful retirement before physical rejection/publication through copied data; focused regressions prove ordering and nonpublication; all current gates pass; retained evidence states exactly whether the stable timing-row identifier is available or transfers. |
| Original Owner Request | Continue the approved T390 prerequisite before DeskPro board timing, use Core for reusable hardware capability, and avoid pure test APIs while keeping external ROM/media out of the repository. |
| Similar-Issue Sweep | Inspect every instruction-record callback, `core_machine_instruction_cost()` return route, source-unallocated marker, physical-rejection path, elapsed publisher and generic trace client for duplicate capture/publication or a missed successful-retirement route. |
## Current Technical Baseline

- **Current developer artifact:** T386 S28 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `80EC4B58630B163ECAFED6FE5B97F3B69D5E22D87928811FE66A8D5274E64EDD`.
  T386 closes selected-device functional completeness at S29; its retained
  [closure audit](../etc/evidence/t386-s29-functional-closure-audit.md) fixes
  HDC current-gate coverage and transfers board, firmware and physical work.
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
| T390 S2 | Accepted: external Model-40 BYOB corpus/observation containment is frozen; current trace cannot expose pre-publication instruction/context or exact-or-unallocated disposition, so a shared Core capture transfers next. No physical profile, board clock or L3 claim. [Evidence](../etc/evidence/t390-s2-model40-byob-observation-contract.md). |
| T389 | Stopped by owner-approved requeue: S1 proves no physical board slice can begin while 80386 successful retirement remains nonphysical; S2 archives the invalid candidate and queues a shared CPU qualification before a fresh board task. No board timing, physical-clock or L3 claim. [Audit](../etc/evidence/t389-s2-stop-and-requeue-audit.md). |
| T388 | Closed: S1--S9 establish the sole publisher, exhaustive exact-or-nonphysical residual disposition and pre-publication physical-contract rejection. No current CPU profile is physically clock-qualified; board work transfers under this restriction. [Closure audit](../etc/evidence/t388-s9-task-closure-audit.md). |
| T387 | Closed: source/owner inventory, exact port-`61h` reconciliation and reference qualification transfer the shared physical-timebase then physical-device/firmware prerequisites before any DeskPro L3 audit. No physical-time or L3 claim. [Closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md). |
| T386 | Closed: S29 independently reconciles every selected Model-40 functional owner/proof, corrects the omitted S26 HDC current-gate registration and transfers only board, firmware and physical-media work to the next candidate/TODO. No L3 claim. [Closure audit](../etc/evidence/t386-s29-functional-closure-audit.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |

## Recent Governance

- **M5 Td S102 P1:** reconciled accepted D4 parity work into the T386 proposal and Queue title, defined the non-preallocated remaining functional receiver order, and confirmed the existing Core/VM/profile architecture authority needs no amendment.
- **M5 Td S101 P1:** added a compact execution-entry table without creating new paths, and made coordinator semantic review explicit after structural documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339 current-source re-audit before DeskPro, reconciled T380 S2 history, and made host/workspace path detection reject the external-assets form while retaining guest DOS-path examples.
