# Project Status

## Current Work

**Active.** M5 T320 S1 establishes the Intel 80386 VM86-to-protected CPL0
exception and IRQ delivery foundation in Coordinated Dual-Session Mode.

## M5 T320 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; M5 T320 S1; Coordinated Dual-Session Mode. The executor twice stopped without a complete P1 after the entry-only packet revision; on 2026-08-11 the owner explicitly overrode the fallback and re-delegated the same uncommitted P1 to the existing `executor` session. On 2026-08-11 the owner also approved the narrow direct-consumer test migration below. The coordinator independently reviews any complete pushed delivery before governance closure. |
| Admission And Approval | The owner approved this first Queue candidate on 2026-08-11 after T319 closed its non-VM86 LGDT/LIDT slice. This prerequisite owns the VM86-to-protected exception/interrupt transfer required before later privileged VM86 instruction families are re-admitted. The owner approved the entry-only revision and re-authorized Coordinated Dual-Session execution on the same date. After the caller sweep exposed direct legacy VM86 matrix consumers, the owner authorized the narrow direct-consumer migration and then authorized continued implementation in the recommended direction: migrate only valid-delivery assertions in direct VM86 consumers (`S47`, `S48`, `S49`, `S50`, `S55`, `S62`, and `S63`) to the new CPL0-delivered contract; retain their invalid-facility terminal boundaries and all non-VM86 assertions. |
| Objective | Implement and prove the 80386 protected-mode path from VM86 to a CPL0 32-bit IDT interrupt gate for synchronous `#GP`, `#UD`, and `#NM` plus external IRQ0: validate the gate/TSS target stack, atomically select TSS `SS0:ESP0`, construct the Intel VM86 transition frame, and enter CPL0 with correct gate effects. Reconcile only direct historical VM86 consumers (`S47`, `S48`, `S49`, `S50`, `S55`, `S62`, and `S63`) where a valid IDT/TSS facility now makes that producer deliver to CPL0. The inverse CPL0 `IRET` transition is explicitly deferred to T320 S2. |
| Non-goals | CPL0 `IRET` return to VM86, VME/PVI, task gates/switches, paging breadth, call gates, VM86 instruction-family completion, arbitrary VM86 IRET/NT/task returns, 16-bit gate breadth, NMI redesign, generic PIC redesign, descriptor-table load forms, and 80387 implementation. The later processor-control package owns VM86 LGDT/LIDT proof. |
| Reference Baseline | `c48d28e8`; current developer artifact remains the closed T319 build until this S emits the allocated T320 artifact. |
| Files And ABI Surface | May change only protected exception/interrupt/IRET delivery paths, their direct stack/TSS/segment helpers if a focused defect proves it, the owner smoke, `core_machine_pushf_popf_smoke.c`, `core_machine_cli_sti_smoke.c`, `core_machine_hlt_s49_smoke.c`, `core_machine_software_int_s50_smoke.c`, `core_machine_port_io_s55_smoke.c`, `core_machine_clts_s62_smoke.c`, and `core_machine_msw_s63_smoke.c` for the owner-approved direct-consumer migration, CMake registration/artifact wiring, T320 evidence/history, and STATUS. No public ABI, provider boundary, generic memory/paging, task-switch, VME/PVI, x87-provider, or unrelated test migration. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/CODING.md`, `docs/rules/ARCHITECTURE.md`, and `docs/rules/DOCUMENT.md`; Intel 80386 protected/virtual-8086 interrupt, exception, TSS, and IRET rules; project type vocabulary and target-local strict GCC rules. |
| Verification | Fresh GCC configure; strict owner target; deterministic owner marker; exact current-gate registration; valid VM86 `#GP`, `#UD`, `#NM`, and IRQ0 paths through a 32-bit CPL0 interrupt gate; full frame/stack/cache/gate-effect assertions; invalid-gate/TSS/target-SS atomicity; for each named direct consumer, valid IDT/TSS VM86 producer cases assert the delivered CPL0 boundary while invalid delivery facilities retain their established terminal diagnostic boundary; documentation governance; diff check; T320 artifact rebuild, identity, and SHA-256; complete current-gates-gcc; commit and push. |
| Expected Markers | One deterministic `M5:T320:S1:VM86-DELIVERY:OK` marker and exactly one `current.core-machine-vm86-delivery-smoke` current-gate registration. |
| Asset Needs | None; deterministic local CPU, GDT, IDT, 32-bit TSS, stack, PIC, and owner-controlled fault fixtures only. |
| Reporting Requirements | The packet was revised before implementation on 2026-08-11 to transfer inverse `IRET` return to the planned S2. The owner then explicitly re-delegated the entry-only P1 and subsequently authorized the named direct-consumer migration to the existing executor session. P1 is pushed but requires the consolidated P2 corrective below before acceptance. Executor supplies the complete corrective implementation/evidence package, self-review, commands, commit, and immediate push; it must not report partial vectors as a delivery. Coordinator independently reviews actual code/tests/CMake/evidence against this packet and then either issues one further consolidated corrective brief or accepts and closes governance. |
| Stop Conditions | Stop rather than broaden if valid VM86 delivery requires a generic paging/memory transaction, task switching, VME/PVI, a provider ABI change, a nonlocal PIC redesign, a semantic change to unrelated real/protected delivery, or a consumer migration outside the named S50/S55/S62/S63 VM86 branches. If a delivery failure class reaches another caller, record the caller sweep and request a revised scope before changing it. |
| Exit Criteria | For all four admitted origins, a valid VM86 source reaches the chosen CPL0 handler through a 32-bit IDT interrupt gate and TSS `SS0:ESP0`; the exact VM86 frame and saved source state are asserted, and IF/TF/gate effects are correct. Each named direct consumer truthfully asserts that valid facilities deliver, while invalid-facility paths retain precise terminal boundaries and non-VM86 coverage remains unchanged. Invalid IDT/TSS/target-SS/stack cases retain pre-publication state or use precisely documented delivered-fault boundaries. No temporary diagnostics remain in owner tests. No inverse VM86 IRET, VME/PVI, task switch, paging, or later VM86 instruction family may be claimed. All required gates, artifact evidence, commit, and push must pass. |
| Original Owner Request | Use Coordinated Dual-Session Mode to implement the next high-ROI 80386 prerequisite after T319, keeping boundaries explicit and code quality/gates complete. |
| Similar-Issue Sweep | Audit every caller and mode branch of `_ser_int_protected`, `_ser_int_protected_32_outer`, `_e_except_n`, `_e_intr_n`, and protected IRET. The direct consumer sweep already identified S47/S48/S49 and the approved S50/S55/S62/S63 set. Dispose each remaining VM86-relevant hit as covered, out of scope with reason, or a TODO/next-package transfer; do not alter unrelated real/protected delivery silently. |

## Current Technical Baseline

- **Current developer artifact:** T320 selects `vm-0-5-0320` /
  `build/output/nxvm_0_5_0320.exe`; its commit, SHA-256, runtime identity, and
  retained boundaries are in [T320 history](history/M5-T320-vm86-delivery.md).
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
| T319 | Closed the bounded non-VM86 LGDT/LIDT `0F 01 /2,/3` table-load slice: protected CPL>0 now rejects before pseudo-descriptor reads, both forms have real/protected, attribute, source-atomicity, table-consumer, and PIC evidence, and VM86 plus 80286 LOCK remain transferred. Its 0319 artifact and 196/196 current-gate pass are retained in history. |
| T318 | Closed the bounded SGDT/SIDT `0F 01 /0,/1 table-store slice: a local six-byte preflight/publication repair prevents partial pseudo-descriptor writes; both forms have profile, attribute, segment, rejection, protected atomicity, VM86 and IRQ-ordering evidence. 195/195 current-gate passed, and executor plus coordinator 0318 artifact hashes are retained in its history. LGDT/LIDT and wider processor-control work remain future bounded packages. |
| T317 | Closed test-corpus quality plus corrective type-vocabulary and developer-feedback work. S7 retains the 194-test current gate while adding validated default-four-job CTest execution, 15 explicit media labels, and a 179-test non-media fast preset; its full gate, fast run, media verifier, documentation governance, and diff check passed. |
| T316 | Closed the declared 80386 ordinary-execution slices through S66. Residual parent-family breadth remains explicitly transferred in the S66 matrix to named later Queue packages, the legacy LOCK TODO, or the external-coprocessor boundary; no whole-80386 claim. Its predecessor developer artifact and 194 current-gate tests passed. |
| T314 | Closed the EGA CRTC 13h bound, registered the deterministic xasm smoke, restored six historical target-local strict GCC sets while retaining three selected libraries, and added the CRTC static closure verifier; the two inherited-governance debts remain. Its 0315 artifact, 52 gates, 147 CTests, and S2/S3/S6 markers passed. |
| T313 | Completed construction-failure atomicity and startup-failure visibility: single RAM allocation, transactional port/controller assembly, visible session start failure, and explicit debugger mappings. Its accepted artifact, 51 gates, 145 CTests, and S2--S6 focused markers passed. |
| T307 | Completed 80386 32-bit CPL3-to-CPL0 IDT and call-gate entry with TSS32 target stacks, preflighted atomic publication, and Intel-correct target-SS `#TS` classification. Its 0307 artifact, 51 gates, 137 CTests, and 9 focused/retained probes passed; product observation remained host-window limited before guest input. |
| T308 | Completed protected 32-bit same/outer error delivery and bounded contributory `#DF` containment through the existing route. Its 0308 artifact, 51 gates, and 137 CTests passed; triple-fault shutdown/reset remains deferred. |

## Recent Governance

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

- **M5 Td S60:** routed every DOS 5-bound smoke contract through the explicit
  M1 FDD image while retaining the current MS-DOS 6.22 FDD and EWIN31 Setup
  HDD for their applicable checks. Fresh configuration restored 145/145 current
  gates without weakening old assertions or claiming broad DOS 6.22 support.
