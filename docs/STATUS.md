# Project Status

## Current Work

**Idle.** M5 T323 S7 is closed; the next protected 80386DX matrix slice
requires a separately admitted packet.

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
| T323 S7 | Accepted protected 16-bit call-gate entry: 80286 TSS16 plus 80386 TSS16/TSS32 outer calls, same-CPL DPL behavior, two-word parameter copy and exact six-word frame, gate/code/TSS/SS/stack/source prepublication boundaries, and STI-composed IRQ ordering are focused-proven. `_ser_call_far_call_gate` now preflights and copies 16-bit parameters; the refreshed 0323 artifact SHA-256 is recorded. Target-local strict GCC, governance, and 209/209 current-gate passed. [Evidence](etc/evidence/t323-s7-protected-16-call-gate.md). |
| T323 S6 | Accepted protected CPL0-to-CPL3 16-bit outer `IRET`: 80286/80386 default plus 80386 `67`, exact five-word frame consumption, cache/CPL/FLAGS/16-bit-SP publication, invalid CS/SS/short-frame atomicity, and restored-IF IRQ composition are focused-proven. No production change; target-local strict GCC, governance, and 208/208 current-gate passed. [Evidence](etc/evidence/t323-s6-protected-16-outer-iret.md). |
| T323 S5 | Accepted protected CPL3-to-CPL0 16-bit external entry: 80286 TSS16 and 80386 TSS16/TSS32 paths publish the five-word outer frame, DPL0 CS/SS stack state, gate-specific IF/TF effects, and IRQ/NMI acknowledgment. Invalid TSS plus null/non-present SS0 retain target-stack/PIC publication boundaries. No production change; target-local strict GCC, governance, and 207/207 current-gate passed. [Evidence](etc/evidence/t323-s5-protected-16-outer.md). |
| T323 S4 | Accepted same-CPL3 16-bit protected external entry: 80286/80386 IRQ0 and NMI bypass DPL-zero interrupt/trap gate software policy, publish the exact three-word frame and gate-specific IF/TF state, and acknowledge their source. Invalid and non-present external gates retain source ownership and target-stack nonpublication after the source NOP. No production change; target-local strict GCC, governance, and 206/206 current-gate passed. [Evidence](etc/evidence/t323-s4-protected-16-external.md). |
| T323 S3 | Accepted same-CPL 16-bit protected interrupt/trap gates: 80286/80386 software frames, 66/67/LOCK classification, software DPL rejection, existing error-code frame, and external IRQ frame are focused-proven. `_ser_int_protected_16` now distinguishes software origin from error frame, accepts trap gates, clears IF only for interrupt gates, and clears TF for both. Target-local strict GCC, refreshed 0323 artifact, governance, and 205/205 current-gate passed. [Evidence](etc/evidence/t323-s3-protected-16-gate.md). |
| T323 S2 | Accepted the protected loaded DS/ES/SS data-access matrix: 80286/80386 cache rights, null/read-only/ordinary/expand-down limits, DS/SS/ES selection, profile/LOCK rejection, and the corrected same-CPL 16-bit IRQ0 FLAGS/CS/IP frame are focused-proven. The sole `_ser_int_protected` call-site repair passes the existing error-frame classification; target-local strict GCC, the 0323 artifact, governance, and 204/204 current-gate passed. [Evidence](etc/evidence/t323-s2-protected-data-access.md). |
| T323 S1 | Accepted the direct protected far `CALL`/`JMP` code-descriptor matrix: 80286/80386 immediate and `FF /3,/5` forms, 66/67 routes, descriptor/RPL/DPL/present classification, target and stack preflight, LOCK/profile rejection, and protected IRQ0 no-shadow are focused-proven. No production change; target-local strict GCC, governance, and 203/203 current-gate passed. [Evidence](etc/evidence/t323-s1-protected-far-transfer.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |
| T321 | Closed the bounded exception, interrupt, return, VM86 table-load, and processor-control composition program: S2 delivers active `#DE/#PF/#MF` vectors; S3 proves NMI/IRQ/TF ordering; S4 composes software INT/IRET with IRQ; S5 enforces VM86 LGDT/LIDT `#GP(0)` before source access; and S6 records the artifact and all transfers. The 0321 artifact, governance, and 202/202 current-gate passed. |
| T320 | Closed the bounded VM86-to-CPL0 32-bit delivery foundation: `#GP/#UD/#NM/IRQ0` entry through TSS `SS0:ESP0`, full VM86 frame and failure boundaries, plus atomic nine-dword CPL0 `IRET` return with a real IRQ0-to-handler-to-VM86 round trip. `66` is classified as non-VM86 return, `67` succeeds, and VME/PVI/task/paging breadth remains transferred. Artifact 0320 SHA-256 is recorded in history; fresh configure, governance/diff checks, and 198/198 current-gate passed. |
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |

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
