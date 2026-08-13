# Project Status

## Current Work

**M5 T338 S1 - active.** Establish the complete 8086/80186 primary-form and
`LOCK` allocation ledger before implementation slices close the legacy profile
boundary.

## M5 T338 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | The owner approved on 2026-08-13 the Queue-ordered single-agent four-profile program, including the 8086/80186 closure after T337. T337 closed in `c2dd549d`; this is the next Queue candidate and begins T338. |
| Objective | Derive one exhaustive 8086/80186 primary-form ledger from Intel profile availability, current metadata/dispatch, existing owner evidence, and T336's transfer map. Classify every inherited or 80186-only form, ModRM extension, defined FLAGS/stack quirk, attribute/profile rejection, and memory-capable `LOCK` disposition as current proof, bounded implementation slice, later-profile transfer, reserved rejection, or external coprocessor boundary. Identify shared decode/prefix/LOCK owners and exact caller/write/validation/commit/rollback paths before any handler repair. |
| Non-goals | No instruction semantic, decoder, prefix, production, fixture, or public-interface change; no 80286 protected, 80386 width/VM86/paging/debug, x87 execution, device/timing, or corpus-only completion claim. This S allocates the legacy task; it does not close a form merely because existing code is reachable. |
| Reference Baseline | `c2dd549d` (`M5 T337 S3 P2: close shared delivery foundations`), retained T337 developer artifact `0.5.0337`, and the T336 coverage ledger. |
| Candidate Proposal | [8086 and 80186 profile closure](../proposals/m5-8086-80186-profile-closure.md); [four-profile CPU-completeness program](../proposals/m5-four-profile-cpu-completeness-program.md). |
| Files And ABI Surface | Expected: T338 history record, Current packet, one indexed form-allocation evidence ledger, and only the task's required developer-artifact identity update. No production or test behavior/API/ABI change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: full-P, mechanism-defect, ledger/transfer, artifact, and closure lifecycle. `docs/rules/ARCHITECTURE.md`: one owner per state transition and no duplicate construction. `docs/rules/CODING.md`: no speculative abstraction. `docs/etc/operations/policy/source-policy.md`: Intel authority only; no reference import. |
| Verification | Enumerate metadata and primary dispatch assignments; inspect every 8086/80186 handler family and existing owner smoke/evidence; search all tracked sources/CMake/docs for `PREFIX_LOCK`, prefix decode, minimum-profile gates, `UndefinedOpcode`, and real-mode `#UD` owners. Rebuild the task developer artifact as `vm-0-5-0338`, run artifact/document governance and diff checks, and record the exact later implementation S boundaries with no unclassified in-scope row. |
| Expected Markers | One form ledger maps every 8086/80186 candidate form to valid/rejected/transferred/external disposition and evidence; one complete LOCK matrix names legal/illegal/profile outcomes for every allocated memory-capable form; each future slice has a mechanism owner, form set, proof, stop condition, and no duplicated profile scope. |
| Asset Needs | Intel 8086/80186/80386 instruction manuals already approved as architectural authority. No guest media, firmware, external source, binary, or trace. |
| Reporting Requirements | Report a material classification conflict before implementation. Otherwise deliver one complete pushed P with the exact inventory commands, form/LOCK ledger, existing-evidence limitations, every later-slice boundary, artifact identity, and governance results. |
| Stop Conditions | Stop if the Intel/profile form set cannot be reconstructed from the authoritative materials and current metadata, if a common decoder/prefix change is required to make the ledger truthful, or if an in-scope form depends on 80286/80386 state. Transfer the row exactly; do not infer semantics or expand a later profile into T338. |
| Exit Criteria | No 8086/80186 form, ModRM extension, LOCK classification, or defined state/fault row is unclassified; every partial row has one bounded subsequent S or later-task/TODO owner; no runtime claim is made; task artifact, documentation governance, and diff gates pass; the P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Search all tracked production, test, build, Queue, TODO, history, and evidence paths for 8086/80186 metadata, `PREFIX_LOCK`, `UndefinedOpcode`, primary dispatch `00`--`FF`, arithmetic/stack/string/I/O forms, and retained legacy-profile smoke evidence. |

## Current Technical Baseline

- **Current developer artifact:** T338 S1 selects `vm-0-5-0338` /
  `build/output/nxvm_0_5_0338.exe`; the rebuilt developer output SHA-256 is
  `322B835AF2E0647A65627023ACD87D50A9F8F6F3BF0D9F95C384D2A8D213214C`.
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
| T337 | Closed shared CPU state and delivery foundations: vector-6 fault delivery/rollback, software and external origin composition, IRQ/NMI/TF priority, frame timing, inhibition, and post-commit task debug trap have one proof/owner. DR6/DR7 transfers to 80386DX; reset/shutdown/triple-fault remains bounded CPU/machine debt. [History](../history/M5-T337-shared-state-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |

## Recent Governance

- **M5 Td S79:** reordered the M5 CPU-completeness program around a
  four-profile audit, shared delivery foundations, 8086/80186, 80286, and
  80386DX closure candidates, then cross-profile verification. Each candidate
  has a linked unnumbered proposal; no implementation task was allocated.
  Documentation governance, Queue-link verification, and diff check passed.

- **M5 Td S77:** moved the closed T323/T325/T328 proposals into matching
  history companions; made every current proposal require a Queue link and
  added the orphan-proposal negative self-test; compacted repeated
  mechanism-defect requirements into role-specific authorities; clarified
  historical terminology retention and the idle T332 artifact wording.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S76:** retired the closed T330 task-switch debt from TODO and the
  unqueued proposal surface; retained its proposal, debt report, history, and
  matrix as explicitly historical/non-current evidence; and corrected the
  stale T330 closure summary. Documentation governance and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S75:** reconciled Queue state with recorded closures: removed only
  the T323 protection/privilege, T325 paging, T328 legacy LOCK, and T330
  transition-unification candidates; retained the remaining candidate order.
  Documentation governance and diff check passed. Td work has no runtime or
  artifact change.

- **M5 Td S74:** clarified that an executor reports a discovered issue while
  the coordinator alone revises the active packet/brief or admits later S
  work, and renamed the architecture status reference to `CURRENT.md`.
  Documentation self-test, combined governance check, and diff check passed.
  Td work has no runtime or artifact change.

- **M5 Td S73:** replaced ordinary/dual-session branches with one
  coordinator/executor role cycle. One session switches roles and cannot claim
  independent review; two sessions obtain independent coordinator review. P,
  Td/T closure, packet, stop, handoff, and role-authority constraints remain
  explicit. Documentation self-test, combined governance check, and diff check
  passed. Td work has no runtime or artifact change.

- **M5 Td S72:** aligned executor-facing governance: the coordinator creates
  active packets; executor findings are reported for coordinator triage; every
  numbered T packet must link its candidate proposal; and Td P1 is complete
  without a later governance P. Positive and missing-proposal negative
  self-tests, the combined governance check, and diff check passed. Td work
  has no runtime or artifact change.

- **M5 Td S71:** adopted the shared `states/` and `proposals/` topology:
  migrated Current, Queue, and TODO; extracted every Queue candidate detail to
  a linked proposal; updated current links and the topology/self-test verifier;
  and made execution the single authority for proposal admission, task
  numbering, history naming, retention, and withdrawal. Only standalone Td
  work may modify `docs/rules/`. Documentation self-test, combined governance
  check, and diff check passed. Td work has no runtime or artifact change.
