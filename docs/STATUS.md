# Project Status

## Current Work

M5 T324 S1 is active: separate current smoke execution from specialized
verification gates (Ordinary Mode).

## M5 T324 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New |
| Admission And Approval | Owner-authorized continuation of the Queue's ordered M5/80386DX program, 2026-08-12. T323 is closed. |
| Objective | Mechanically separate developer smoke execution from specialized verification: `run-current-smokes` builds registered smoke executables and runs only the full `current-gate` CTest selection; `run-current-fast-smokes` remains the non-media selection; one dedicated verifier target owns all structural, provenance, static, and other `verify-*` checks; and `current-gates-gcc` composes the three layers without duplicate execution. |
| Non-goals | No registered test, command, assertion, timeout, media classification, artifact contract, product behavior, source ABI, verifier semantics, or CTest registration may be removed, relabeled, or weakened. Do not make static verifiers into CTests or change tests only to alter timing. |
| Reference Baseline | `7fde9768`; the Queue P1 current-test/specialized-gate candidate and T317 S7 smoke-scheduling evidence are inputs. |
| Files And ABI Surface | Expected: CMake presets/target composition, a narrow mechanical verifier or evidence, Status, and T324 history/evidence. No product source, public API, fixture behavior, media asset, or test executable source change is admitted. |
| Applicable Rules | Task Reading Set; execution/coding/documentation rules; Queue; CMake/CTest target graph; T317 S7 evidence; current artifact contract. |
| Verification | Audit every existing `run-current-smokes` dependency and classify it. Prove exact full/media/non-media CTest counts and partition; prove every specialized verifier is absent from pure smoke and present in complete acceptance; verify no duplicate target execution in the complete graph; record elapsed baselines for full, fast, specialized, and aggregate gates; run documentation governance and diff check. |
| Expected Markers | `M5:T324:S1:CURRENT-GATE-SEPARATION:OK`; `run-current-smokes`, `run-current-fast-smokes`, and `verify-current-specialized-gates` have distinct mechanical target roles. |
| Asset Needs | Existing owner-provided local media only; no new asset, firmware, guest media, or network input. |
| Reporting Requirements | Deliver one complete implementation P with full dependency classification, implementation, evidence, and all verification while retaining this packet. After push, perform ordinary-mode actual-change acceptance before a governance P closes S1. |
| Stop Conditions | Stop if a verifier has an undocumented runtime dependency, accurate separation changes test/product behavior, a target cannot be classified without changing its contract, or CMake cannot prove the complete graph is non-duplicating. Revise the packet rather than weaken coverage. |
| Exit Criteria | Full/fast CTest selections and media partition are exact; every specialized verifier has one defined owner target; pure smoke contains no verifier dependency; complete acceptance contains every verifier and smoke once; timing evidence is recorded; no scope drift; all gates pass; and a pushed governance P records acceptance. |
| Original Owner Request | Complete the owner-approved Intel 80386DX architecture program with evidence-led code quality and package governance before M6. |
| Similar-Issue Sweep | Review `PROJECT_CURRENT_*_SMOKE_TARGETS`, `project_add_current_smoke_test`, every `add_dependencies(run-current-smokes ...)`, `verify-*` target, `current-gates-gcc`, fast presets, CTest labels, media classification, and current artifact verification. |

## Current Technical Baseline

- **Current developer artifact:** T323 selects `vm-0-5-0323` /
  `build/output/nxvm_0_5_0323.exe`; its source commit, SHA-256, and bounded
  protection/privilege closure transfers will be recorded at T323 closure.
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
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |

## Recent Governance

- **M5 Td S68:** reconciled open numeric-task progress with identifier
  allocation: commit history proves used S/P records, Status structural rows
  determine task closure, and the verifier permits only the exact next S of the
  latest retained-open task under `Continuation`. Added positive and
  skipped/foreign/missing-progress/new-task negative self-tests. Documentation
  self-test, default check, and diff check passed. Td work has no runtime or
  artifact change.

- **M5 Td S67:** defined the two complementary 80386DX completion views in
  goal, roadmap, Queue, and an indexed supporting closure map; retained the
  T316 form matrix as horizontal evidence, and recorded VME/PVI, x87, and
  legacy LOCK boundaries without adding project-specific content to global
  rules. Documentation and state checks plus diff check passed. Td work has no
  runtime or artifact change.

- **M5 Td S66:** replaced all-documents startup reading with a role- and
  change-triggered reading set; aligned implementation and governance P
  lifecycle in both execution modes; clarified owner approval, coordinator and
  executor reporting, S-brief authority, corrective re-admission, and Status
  retention; added the `Reporting Requirements` packet field; and split the
  governance verifier into diagnostic documentation and state scopes while
  retaining its combined closure gate. Documentation and state checks, combined
  check, self-test, and diff check passed. Td work has no runtime or artifact
  change.

- **M5 Td S65:** aligned active numeric-task progress retention with task-level closure consolidation, added narrow structural checker coverage (including retained-progress self-tests), and preserved the eight-row cap for task-level closures. Td work has no runtime or artifact change.

- **M5 Td S64:** requires an actual-change review before a task or subtask is
  accepted: reports, test summaries, and diff statistics are evidence indexes,
  not substitutes for reading the relevant changed code, build, test, and
  documentation artifacts. In dual-session mode the coordinator owns that
  independent review. Td work has no runtime or artifact change.

- **M5 Td S63:** made Intel 80386 PRM form audits a per-candidate admission
  input and exit gate. A bounded task can close only its declared matrix slice;
  a family cannot close while any in-scope form is partial, missing, or
  unclassified. Td work has no runtime or artifact change.

- **M5 Td S62:** recorded the owner-approved Intel 80386DX
  architecture-completeness program before M6, replaced the closed T314 Queue
  candidate with its ordered family sequence, scoped 80386-to-x87 coupling
  without admitting an 80387, and recorded the required reuse and coverage
  discipline. Td work has no runtime or artifact change.

- **M5 Td S61:** restored Queue dependency order by placing residual M5 work
  before M6 mantle candidates; removed stale package/index detail; and changed
  historical evidence to name closure artifacts rather than current state.
