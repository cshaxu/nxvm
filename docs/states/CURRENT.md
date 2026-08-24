# Project Status

## Current Work

**Active: M5 T450 S3.**

## M5 T450 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | Owner: repository user. Continuing the approved per-controller paired T450 plan. Scope: PIC 8259A original source/function/timing checklist only; S4 owns its current-code gap audit. |
| Objective | Record the complete 8259A register, initialization/operation command, priority/cascade/interrupt lifecycle, reset and admissible timing universe from the admitted Intel and IBM sources. |
| Non-goals | Do not audit current PIC implementation or tests, change code, infer INTA waveform/board timing, or create non-PIC checklist rows. |
| Reference Baseline | T450 S1 source ledger, T450 S2 controller-subtask ledger, Intel 8259A 231468-003 and IBM 5170 technical reference. |
| Candidate Proposal | [Core controller manual admission and gap inventory](../proposals/m5-core-controller-manuals-and-gap-inventory.md). |
| Files And ABI Surface | Indexed `docs/etc/research/` only; external manuals are read-only; no product source, ABI or artifact change. |
| Applicable Rules | `docs/README.md` Task Reading Set; `docs/rules/EXECUTION.md` finite-ledger/actual-review requirements; `docs/rules/DOCUMENT.md` supporting-detail boundary; source policy; no production architecture or coding change. |
| Verification | Reconcile every 8259A command/register/mode family against the source table/section pages; verify each timing/board fact has a source or explicit L2 boundary; documentation governance gate and `git diff --check` pass. |
| Expected Markers | One PIC-only checklist has source page, function, timing, reset/cancellation, IRQ/cascade relation and L3/L2/blocked disposition for all finite 8259A families. |
| Asset Needs | T450 S1 Intel 8259A and IBM 5170 external PDFs only. |
| Reporting Requirements | Record exact Intel and IBM page references, the finite PIC universe, every source-limited timing disposition and S4 transfer. |
| Stop Conditions | Mark a row L2 when selected IBM board timing cannot be cited; never infer INTA waveform or another controller's route. |
| Exit Criteria | The indexed PIC original checklist covers all 8259A command/register/mode/lifecycle families once and only once and transfers unchanged row identifiers to S4. |
| Original Owner Request | One controller S establishes its original function/timing checklist; the following S audits its current-code gaps. |
| Similar-Issue Sweep | Compare master/slave cascade, IRQ source, CPU acknowledgement and reset facts within PIC only; do not inspect implementation gaps. |

## Current Technical Baseline

- **Current developer artifact:** target `vm-0-5-0448`; `nxvm_0_5_0448.exe` / `build/output/nxvm_0_5_0448.exe`, SHA-256 `A89C1901B3D631DFD7482432E74C4CB799BB2A549FA5C6BE93F7205A0BB83677`. T434 has one copied Core timing-plan publication route for default PC/AT, IBM 5170 Model 339 and Model-40 BYOB session composition.
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
| T450 S2 | Accepted: one original-list S and one gap-audit S are fixed for every controller; S19 reconciles the eight pairs without adding behavior. [Ledger](../etc/evidence/t450-s2-controller-subtask-ledger.md). |
| T449 | Closed: one copied transaction contract, transaction lifecycle, scheduler arbitrator, memory classifier and retirement/time/observation route; all S1--S6 evidence is reconciled and physical/controller/x87 boundaries transfer explicitly. [History](../history/M5-T449-core-cpu-board-transaction-contracts.md). |
| T448 | Closed: six over-limit generated firmware sources now use the one bounded byte-identical materialization route; the unused session helper is removed. [Evidence](../etc/evidence/t448-s1-firmware-materialization.md). |
| T447 | Closed: all S1--S11 ledger receivers are accepted; one Core execution path, bounded VM contracts, no VDM forwarding facade, owner-local test boundaries and the decomposed Core coordinator are retained. [History](../history/M5-T447-architecture-boundary-debt-closure.md). |
| T446 | Closed: each VM session owns its debugger cursor state; the sole recorder owner closes on stop, write failure and destruction, clearing failed streams and reporting lifecycle outcomes. [Evidence](../etc/evidence/t446-s1-vm-debugger-recording-lifecycle.md). |
| T445 | Closed: each VM native display adapter now owns its host resource lifecycle; Win32 pairs its DC before window destruction, and Linux curses terminates on the initializing display thread. [Evidence](../etc/evidence/t445-s1-vm-native-display-resource-lifetime.md). |


## Recent Governance

- **M5 Td S139 P1:** inserted one source-admission and implementation-gap
  inventory candidate ahead of the eight controller implementations; it
  requires two finite checklists per controller and makes no source or runtime
  change. [Evidence](../etc/evidence/td-s139-controller-manual-inventory-admission.md).

- **M5 Td S138 P1:** replaced the aggregate controller/device implementation
  candidate with ordered PIC, DMA, PIT, RTC, KBC, FDC/media, VADP and
  source-blocked HDC proposals; the aggregate now supplies shared context only.
  [Evidence](../etc/evidence/td-s138-controller-phase-program-decomposition.md).

- **M5 Td S137 P1:** clarified the queued CPU-to-board transaction candidate's
  sole-owner data flow, direct-path removal constraint, L2/rejection boundary,
  and bounded S1--S6 implementation sequence; no runtime behavior or Queue
  order changed. [Evidence](../etc/evidence/td-s137-core-transaction-proposal-clarification.md).

- **M5 Td S136 P1:** independently rebuilt the complete Td S135 revision with
  WinLibs UCRT GCC and Ninja (108/108), recorded its output hash, and removed
  the temporary verification tree. [Evidence](../etc/evidence/td-s136-t447-compile-verification.md).

- **M5 Td S135 P1:** removed the two unnecessary VM-to-Core device
  implementation includes and completed T447's history/proposal/status
  migration. [Evidence](../etc/evidence/td-s135-t447-closure-reconciliation.md).

- **M5 Td S134 P1:** created the coverage-bearing architecture-boundary debt
  closure proposal, which admitted and closed as T447; its retained companion
  is now in [T447 history](../history/M5-T447-architecture-boundary-debt-closure.md).

- **M5 Td S133 P1:** audited actual T438--T443 owner paths, focused regressions,
  artifacts, commits and topology; it archived the two retained completed
  proposals and transferred T344 plus 20 current-fast failures to the queued
  current-gate restoration candidate. [Evidence](../etc/evidence/td-s133-recent-task-closure-audit.md).

- **M5 Td S132 P1:** republished the generated T435 S5 8086 result artifact
  after current Core-private source-form IDs evolved. The runner and both
  8086 result/decoder gates pass; all 1,053 records retain identical
  non-form-ID observations. [Evidence](../etc/evidence/td-s132-8086-derived-result-reconciliation.md).

- **M5 Td S131 P1:** admitted nine bounded repair candidates ahead of future
  M5 capability work, promoted firmware materialization from TODO, and refined
  the queued YAML candidate's grammar/selection acceptance surface. The
  documentation-governance gate passes; this changes no runtime behavior.

- **M5 Td S130 P1:** sixth-pass manual review of selected VM production
  dependencies on Core-private machine headers confirms a keyboard scan-set
  contract gap and five unused include residues. The indexed
  [evidence](../etc/evidence/td-s130-sixth-pass-audit.md) records exact paths,
  bounded non-findings, and the existing TODO transfer; this is not a
  whole-repository completion or runtime-correctness claim.

- **M5 Td S129 P1:** fifth-pass manual review of Core product-session payloads,
  selected machine state headers, debug/execution providers, and utilities
  confirms raw mutable option/command storage crosses the public session
  boundary. The indexed [evidence](../etc/evidence/td-s129-fifth-pass-audit.md)
  records bounded non-findings and remaining scope; the repair extends the
  existing session-manager boundary task rather than duplicating it. This is
  not a whole-repository completion or runtime-correctness claim.

- **M5 Td S128 P1:** fourth-pass manual review of VM composition, profile,
  product, and public-contract implementation confirms mutable VM-product
  console/catalog state escapes, a test-only duplicated Model-40 construction
  path, and a composition-to-product reverse adapter dependency. The indexed
  [evidence](../etc/evidence/td-s128-fourth-pass-audit.md) also records bounded
  lifecycle/profile-direction non-findings and remaining scope; all repairs
  extend existing non-overlapping TODO admissions. This is not a
  whole-repository completion or runtime-correctness claim.

- **M5 Td S127 P1:** third-pass manual review of CMake platform/source
  ownership, duplicate product command authority, and Core/VM test boundaries
  confirms four additional concrete findings, including systemic VM test
  access to embedded Core state. Named allocator-failure source builds are
  separately confirmed as narrow, recorded test instrumentation rather than a
  new production path. The indexed
  [evidence](../etc/evidence/td-s127-third-pass-audit.md) retains exact
  paths, transfers, and remaining audit limit; this is not a whole-repository
  completion or runtime-correctness claim.

- **M5 Td S126 P1:** second-pass manual review of Core stateful contracts,
  collaborator and plan endpoints, Core debugger context, VM/VDM equivalent
  headers, and host resource/input paths records five additional concrete
  architecture/coding findings. The indexed
  [evidence](../etc/evidence/td-s126-second-pass-audit.md) records the exact
  source paths, one positive input-route disposition, and remaining scope; all
  repairs are transferred to TODO. This is not a whole-repository completion or
  runtime-correctness claim.

- **M5 Td S125 P1:** manually audited the current authorities, source owners,
  composition paths, public contracts, platform adapters, VDM skeleton, tests,
  CMake declarations, and corroborating structural gates. The indexed
  [audit evidence](../etc/evidence/td-s125-manual-architecture-coding-audit.md)
  records nine concrete architecture/coding findings and explicit non-findings;
  all repair work is bounded in TODO. This is not a runtime-correctness claim.

- **M5 Td S124 P1:** ran limited static dependency and C-facade checks. Those checks passed, but they were not a full manual architecture/code audit and do not establish absence of invariant violations; Td S125 performs the required source and documentation review.
