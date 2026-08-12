# Project Status

## Current Work

**M5 T325 S3: close the Intel 80386DX paging and translation package (Ordinary Mode).**

## M5 T325 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; Ordinary Mode. `M5 T325 S3` follows accepted S1 and S2 in the sole open numeric task. |
| Admission And Approval | Owner-approved autonomous continuation of the Queue's 80386DX paging and translation closure on 2026-08-12. This final S admits the package-level form/state audit and bounded post-80386 invalidation rejection proof only. The S3 audit found protected-mode `#UD` IDT delivery absent from the shared finalizer; that separately tracked exception-delivery defect is transferred rather than silently absorbed. |
| Objective | Close T325's native Intel 80386 non-PAE paging/translation package: reconcile CR0/CR2/CR3, 4-KiB PDE/PTE walk, U/S and R/W protection, A/D publication, cross-page atomicity, delivered `#PF`, and the no-persistent-cache model. Add a focused `0F 01 /7` `INVLPG` rejection proof for 80386's pre-486 boundary: metadata rejection occurs before effective-address decoding, and real-mode execution retains complete CPU state. |
| Non-goals | No PAE, PSE, CR4, `INVLPG` implementation, persistent TLB/timing model, TR6/TR7, task-switch CR3 loading, VM86 paging-family breadth, page-table ABI, host-memory replacement, public API, generic exception/PIC redesign, or x87 work. |
| Reference Baseline | `0e308e49` / `vm-0-5-0323`; T258/T311 retained walk and permission probes, T321 delivered-`#PF`, and T325 S1/S2 evidence are inputs requiring actual source/evidence reconciliation. |
| Files And ABI Surface | Expected: existing owner paging smoke, paging closure map/audit/history, and task Status. Production change only for a reproduced local 80386 page-walk or `0F 01 /7` classification defect; no public ABI or memory-route change. |
| Applicable Rules | `docs/design/GOAL.md` goal 6; Queue paging candidate; closure-map paging row; Architecture Rules single CPU executor/page-walk owner; Coding Rules project types and test-only fixture boundary; Documentation and Execution Rules packet/P/task-closure lifecycle. Intel 80386 PRM chapters 5, 6, 9, and 10 are form/state authority. |
| Verification | Focused paging smoke extends with `0F 01 /7` 80186/80286/80386 real-mode `#UD` and complete-state nonpublication, plus 80386 `66`/`67`/combined/LOCK boundaries; source audit confirms metadata rejects `/7` before the table-memory decoder. Existing CR0, permission, cross-page, CR2/CR3, and delivered-`#PF` vectors run. Reconcile each native 80386 paging requirement to executable evidence or a named transfer; configure, exact registration, specialized verifier aggregate, governance, diff, and full current-gate pass. |
| Expected Markers | `M5:T325:S3:PAGING-CLOSURE:OK` alongside retained T258/T311/T325 paging markers; specialized verifiers and full current-gate succeed. |
| Asset Needs | None; deterministic in-memory page tables, descriptor/IDT state, and owner-local fixtures only. |
| Reporting Requirements | Before code, retain the form/state closure ledger and similar-issue sweep. P1 is one complete pushed implementation/evidence/closure-map delivery; then perform Ordinary-Mode actual-change review before a pure governance P2 that writes the T325 history and closes S3 and T325. |
| Stop Conditions | Stop before adding a persistent TLB, later-CPU `INVLPG`, task-switch state, VM86 paging breadth, a public interface, or generic exception delivery. The reproduced protected-mode `#UD` delivery gap is outside this paging package and must receive a distinct Queue/TODO transfer; any other shared-route defect requires a revised packet or later task. |
| Exit Criteria | Every native 80386 paging requirement in this package is reconciled to focused evidence; `INVLPG` is proven as a pre-486 `#UD` with metadata rejection before EA decode and real-mode complete-state nonpublication; residual protected-`#UD` delivery, task/VM86 paging, and persistent-cache/test-register work has a precise Queue/TODO transfer; the T325 task-level audit, focused/retained regressions, current gate, specialized verifiers, governance, and diff checks pass. |
| Original Owner Request | Complete Intel 80386 against the Intel form--implementation--test matrix; repair omissions, perform boundary/differential review, and close package evidence before M6. |
| Similar-Issue Sweep | Sweep all CR0/CR2/CR3 users, page-walk and A/D publish paths, `#PF` producer/delivery evidence, `0F 01` reserved branches and pre-386 `0F` dispatch, translation-cache assumptions, current paging tests, Queue transfers, and closure-map statements. |

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
| T325 S2 | Accepted the bounded Intel 80386 CR2/CR3 translation audit: retained control-register form and delivered-`#PF` evidence is reconciled with a two-directory guest CR3 reload proof and a direct current-PTE edit proof with no CR3 reload. The executor intentionally has no persistent TLB; persistent cache timing, `INVLPG`, TR6/TR7, task CR3 loading, and VM86 paging remain named transfers. Focused regressions, 47 specialized verifiers, governance/diff checks, and 209/209 current-gate passed. [Evidence](etc/evidence/t325-s2-cr2-cr3-translation-audit.md). |
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
