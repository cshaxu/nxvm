# Project Status

## Current Work

**M5 T325 S2: reconcile Intel 80386 CR2/CR3 translation and diagnostic semantics (Ordinary Mode).**

## M5 T325 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; Ordinary Mode. `M5 T325 S2` follows accepted S1 in the sole open numeric task. |
| Admission And Approval | Owner-approved autonomous continuation of the Queue's 80386DX paging and translation closure on 2026-08-12. This S admits the remaining bounded CR2/CR3 consumer and translation-cache contract only. |
| Objective | Reconcile Intel 80386 `MOV CR2/CR3` privilege, fixed-32-bit form, CR3 page-directory selection, `#PF` CR2 diagnostic, and CR3 reload invalidation contract with the CPU executor. Prove the implementation's deliberate no-persistent-TLB model never retains a stale translation. |
| Non-goals | No PAE, PSE, 80486 `INVLPG`, CR4, 80386 TR6/TR7 test-register model, persistent TLB or timing model, task-switch CR3 loading, PE-clear/re-entry, page-table ABI, host-memory replacement, public API, generic exception/PIC redesign, or VM86 paging breadth. |
| Reference Baseline | `c5459a85` / `vm-0-5-0323`; retain T258/T311/T321 and T325 S1 paging and delivered-`#PF` probes as evidence inputs, not as unreviewed authority. |
| Files And ABI Surface | Expected: one or more existing owner paging/control smokes, their already registered target-local strict-GCC targets if changed, paging evidence/closure map, and task Status. Production changes only if a reproduced CR2/CR3 or page-walk defect is local to those routes; no public ABI change. |
| Applicable Rules | `docs/design/GOAL.md` goal 6; Queue paging candidate; closure-map paging row; Architecture Rules CPU executor owns page walk; Coding Rules project types, target-local strict compilation, and test-only fixture boundary; Documentation and Execution Rules packet/P lifecycle. Intel 80386 PRM Chapters 4, 5, 9, and 10 are form authority. |
| Verification | Focused real/protected CPL0 and protected CPL3/VM86 classification; CR2/CR3 read-write, reserved/register-only and 66/67/LOCK forms; two distinguishable page directories through guest CR3 reload; page-table edit visibility/no stale translation; delivered `#PF` CR2 and frame evidence; current registration, configure, governance, diff, full current-gate, and specialized verifier aggregate. |
| Expected Markers | Existing focused paging/control markers extended with `M5:T325:S2:CR2-CR3-TRANSLATION:OK`; retained T258/T311/T321 paging and exception markers; full current-gate and specialized-verifier success. |
| Asset Needs | None; deterministic in-memory page-directory, table, GDT, IDT, and PIC fixtures only. |
| Reporting Requirements | Before code, record the Intel form audit, no-cache/TLB boundary, caller sweep, and any material objection. P1 is one complete pushed implementation/evidence delivery; then perform Ordinary-Mode actual-change review before a pure governance P2. |
| Stop Conditions | Stop before adding a persistent TLB, TR6/TR7 semantics, task-switch state, a public interface, generic exception delivery policy, or any requirement beyond Intel 80386. A defect requiring a shared route outside `MOV CRx` or page-walk ownership requires a revised packet or later S. |
| Exit Criteria | CR2/CR3 forms and privilege/attribute boundaries are explicitly classified; guest CR3 reload selects the new directory and no stale translation remains; `#PF` CR2 evidence retains its handler/frame boundary; touched callers are swept; focused and retained regressions, current gate, specialized verifiers, governance, and diff checks pass; TLB/test-register/task transfers are named. |
| Original Owner Request | Continue in single-session mode through the Intel 80386DX architecture-coverage closure audit, prioritizing correct, bounded architectural implementation and evidence. |
| Similar-Issue Sweep | Sweep all CR2/CR3 readers/writers, direct control-register forms, translation-cache assumptions, page-walk page-table reads, `#PF` diagnostic publication, retained T258/T311/T321 claims, and their focused tests; classify every production hit as fixed, applicable evidence, or a named later transfer. |

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
| T325 S1 | Accepted the Intel 80386 CR0/page-protection correction: `MOV CR0` now writes `MP`/`EM`/`TS`/`ET`; ET write semantics were re-audited against Intel 80386 section 11.1.1; stale 80486 `WP` CPL0 write faults were removed while CPL3 U/S/R/W protection remains. The focused paging marker, 209/209 current-gate, specialized verifiers, governance, and diff check passed. [Evidence](etc/evidence/t325-s1-cr0-paging-control.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |

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
