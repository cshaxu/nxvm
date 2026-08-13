# Project Status

## Current Work

M5 T330 S2 - 32-bit call-gate same-CPL convergence (Ordinary Mode). Repair the
proven common privilege-rule and same-CPL construction drift without merging
Intel-required 16/32-bit gate and frame layouts.

## M5 T330 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; Ordinary Mode; owner-authorized T330 S2 after accepted T330 S1. |
| Admission And Approval | Owner approved T330 on 2026-08-12 and required that all analogous accidental 16/32-bit construction divergences found by the whole-file audit be resolved in T330. S1 identified this bounded call-gate drift and reserved it explicitly; S2 consumes that approved finding without widening into generic privilege or exception redesign. |
| Objective | Make the 32-bit protected CALL-gate route obey the shared Intel target privilege rule (`target DPL <= CPL`) and construct the same-CPL frame without requiring a TSS. Retain distinct 16/32-bit parameter and frame layouts, validation, and checked-stack preflight. |
| Non-goals | No task-gate, far-JMP-gate, outer-CPL call-gate, VME/PVI, VM86, generic TSS, generic exception/IRQ, public ABI, or broad descriptor-helper redesign. Do not merge 16/32 frame layouts or relax a privilege check beyond the Intel CALL-gate rule. |
| Reference Baseline | `82bea776` / `vm-0-5-0329`; accepted T330 S1 inventory at `docs/etc/evidence/t330-width-path-inventory.md`; Intel 80386 protected CALL-gate semantics; and retained call-gate focused evidence. |
| Files And ABI Surface | Private `cpu_instructions.c` CALL-gate serializer and an existing focused call-gate owner smoke or a new owner smoke if needed, plus narrow evidence/Status updates. No public API, provider, generic descriptor, or TSS interface change. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/DOCUMENT.md`; preserve checked-memory, descriptor/cache ownership, and project type vocabulary. |
| Verification | Audit both serializers and callers. Prove 32-bit same-CPL CALL-gate success without a valid TR/TSS, equal-DPL target acceptance, lower-target-CPL TSS stack-switch retention, rejected higher-DPL/nonpresent/invalid target boundaries, exact 32-bit frame/parameter publication, prefix/LOCK classification, and pending-IRQ ordering. Run focused, configure, exact current registration, documentation governance, diff check, and full current-gate. |
| Expected Markers | Retain existing call-gate markers and add or extend one owner marker that names T330 S2 CALL-gate same-CPL proof; inventory evidence records the caller/write/fault sweep and retained layout distinction. |
| Asset Needs | No guest media, firmware, external source import, or external network asset. |
| Reporting Requirements | Report only a complete implementation P or material stop condition, mapping the privilege comparison, TSS dependency branch, stack/frame writes, callers, faults, and gate result to evidence. |
| Stop Conditions | Stop for a required generic TSS/descriptor/checked-stack/exception redesign, a caller requiring a distinct unmodeled privilege transition, an Intel semantic conflict, or a correction crossing into a separate VM86/outer-CPL package. Record the transfer rather than symptom-patching. |
| Exit Criteria | The 32-bit same-CPL route accepts an equal-DPL nonconforming target, requires no TSS unless a lower-CPL stack transition needs one, and preflights/publishes the correct 32-bit frame atomically. Both serializers retain Intel-required frame differences; caller and fault boundaries are evidenced, the inventory is updated, and all gates pass. |
| Original Owner Request | Holistically audit and repair accidental 16/32-bit construction drift throughout `cpu_instructions.c`, preserving genuine Intel layout differences. |
| Similar-Issue Sweep | Compare `_ser_call_far_call_gate` and `_ser_call_far_call_gate_32` branch-by-branch: gate/target privilege checks, TSS dependency, parameter copy, frame preflight, descriptor writes, stack writes, and CPU/cache publication. Sweep `_e_call_far` callers and retained CALL-gate smokes; classify any new non-layout divergence in the inventory. |

## Current Technical Baseline

- **Current developer artifact:** T329 selects `vm-0-5-0329` /
  `build/output/nxvm_0_5_0329.exe`; commit `ae91e592` SHA-256 is
  `87982567ACDAC83253A8F6102330F5976B150F2C5AD3CB926D0D6BE4AA41C069`.
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
| T330 S1 | Accepted progress: unified all 286/386 task-transition width pairs behind a private plan; preserved real TSS layouts, completed the 286 LDTR write/preflight, and corrected null non-stack selector materialization. Evidence: [T330 inventory](etc/evidence/t330-width-path-inventory.md); 211/211 current-gate. |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |
| T322 | Audited and withdrew the duplicate ordinary-execution/FLAGS candidate: T316's accepted S23--S65 owner smokes already cover the transferred Intel 80386 ordinary application forms. Remaining work is explicitly protection/privilege, paging, task/debug/VM86, legacy LOCK, or external x87 scope; no invented implementation slice or artifact was created. Documentation governance and diff checks passed. |

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
