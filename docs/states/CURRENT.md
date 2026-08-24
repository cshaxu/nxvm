# Project Status

## Current Work

**Active: M5 T450 S13 - uPD765 FDC and logical-media original function and timing checklist.**

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation - M5 T450 S13; the next exclusive FDC/media batch after accepted S12. |
| Admission And Approval | Repository owner approved T450's paired source-list/code-audit work and requires every checklist 1 to retain source-PDF form and primary-manual sufficiency fields for S19. S13 uses only the S1-admitted NEC uPD765 and IBM AT sources. |
| Objective | Create one finite source checklist for uPD765 command/status/result and mode transitions, DRQ/IRQ/reset timing, selected IBM AT DMA2/IRQ6/drive bindings, and the explicitly missing logical-media/drive facts. |
| Non-goals | No FDC/media/runtime/test change; no firmware/media import; no selection of an unadmitted disk image format, drive personality, motor/rotation formula or external emulator reference. |
| Reference Baseline | S1 admitted NEC uPD765A/uPD765B manual and IBM 5170 Technical Reference; accepted S2 ledger, T433/T449 evidence and existing FDC/media owner paths only as context. |
| Candidate Proposal | [M5 Core controller manual admission and gap inventory](../proposals/m5-core-controller-manuals-and-gap-inventory.md). |
| Files And ABI Surface | Add only one indexed S13 research checklist; inspect admitted PDFs and existing sources without code, test, ABI or asset changes. |
| Applicable Rules | `docs/README.md` fixed reading set; `EXECUTION.md` continuation/coverage/P/closure review rules; `DOCUMENT.md` authority/index/state rules; source policy governs research and forbids third-party or media import. Architecture/coding rules are inspection criteria only. |
| Verification | Read admitted manual/board pages; enumerate command/result/mode/terminal states, timing formula or explicit absence, reset, DRQ/IRQ and board/media relations; record source PDF form and manual-sufficiency gaps; run diff check and documentation governance before P1/P2. |
| Expected Markers | One finite checklist keeps FDC chip behavior separate from host media/drive policy, assigns no unselected medium geometry as fact, and leaves each non-primary timing/media need explicitly L2, L4 or blocked. |
| Asset Needs | None; inspect the admitted owner-managed PDFs only. No disk, firmware or third-party source becomes a repository asset. |
| Reporting Requirements | Record source pages/form, finite function, reset/cancellation, timing/absence, DRQ/IRQ/board relation, manual sufficiency and L3/L2/L4/blocked disposition; report concise source boundary. |
| Stop Conditions | Stop for a needed original drive/media/controller source absent from S1, a source conflict, ambiguity that needs owner profile selection, or a requested runtime change; transfer rather than expand. |
| Exit Criteria | The source checklist covers uPD765 phases and command families, selected AT bindings and all missing media/drive inputs; source-form/sufficiency fields are present; it is indexed, reviewed, committed, pushed and accepted for S14. |
| Original Owner Request | Establish original manual archives, complete function/timing checklist 1 and current-code gap checklist 2 for all eight controllers, with S19 validating PDF form and manual sufficiency. |
| Similar-Issue Sweep | Cover data-rate specification, command/result FIFO, MSR, DOR/DSR/CCR, DMA/IRQ, reset/Sense, drive select/motor/seek/read/write/format/read-ID paths, scheduler advance, media registry/image geometry and selected PC/AT/Model-40 composition boundaries. |

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
| T450 S12 | Accepted: the 16-row KBC audit keeps one KBC owner and transfers only source, serial, status, phase and extension gaps; owner-required S19 PDF-form/manual-sufficiency reconciliation is now mandatory for every checklist 1. [Audit](../etc/evidence/t450-s12-kbc-8042-nmi-code-gap-audit.md). |
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
