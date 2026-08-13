# Project Status

## Current Work

M5 T330 S3 - developer-artifact refresh (Ordinary Mode). Produce and verify the
required current `0.5.0330` developer artifact for the accepted T330 CPU-path
changes; no runtime behavior change is admitted.

## M5 T330 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation; Ordinary Mode; owner authorized complete execution of T330, and S3 closes the task-artifact obligation omitted by accepted S1/S2 implementation commits. |
| Admission And Approval | The owner authorized full single-session completion of T330 on 2026-08-12. Closure review found the mandatory developer-artifact record absent; this bounded continuation produces it without changing the accepted CPU semantics. |
| Objective | Make `vm-0-5-0330` the sole current developer artifact target, build and copy `nxvm_0_5_0330.exe`, and record its source commit, SHA-256, version/banner classification, and verification in T330 history. |
| Non-goals | No CPU, emulator, test, ABI, provider, dependency, source-layout, Queue, TODO, or product behavior change. Do not regenerate or relabel historical artifacts. |
| Reference Baseline | `263766f0` / `vm-0-5-0329`; accepted T330 S1/S2 evidence at `docs/etc/evidence/t330-width-path-inventory.md`; task-artifact requirements in `docs/rules/EXECUTION.md`. |
| Files And ABI Surface | CMake current-artifact declaration and build preset, T330 Status/history records, plus the ignored local `build/output/nxvm_0_5_0330.exe`. No production source or public interface. |
| Applicable Rules | Task Reading Set; `docs/rules/EXECUTION.md`, `docs/rules/DOCUMENT.md`, `docs/rules/CODING.md`, `docs/design/CODING.md`; task-level artifact identity and build-tree hygiene apply. |
| Verification | Fresh GCC configure; build `vm-0-5-0330`; run `verify-current-artifact-target`; inspect executable version/banner where available; calculate SHA-256; run documentation governance, diff check, focused retained T330 smokes, and full current-gate. |
| Expected Markers | `verify-current-artifact-target` selects only `vm-0-5-0330`; retained T330 S1/S2 smoke markers pass; history records the SHA-256 and current-artifact classification. |
| Asset Needs | No guest media, firmware, network asset, or imported source. The output EXE remains ignored local developer evidence under `build/output/`. |
| Reporting Requirements | Report a complete artifact P or a material build/identity blocker, including exact artifact path, source commit, SHA-256, target selection, retained smoke/gate result, and build-tree disposition. |
| Stop Conditions | Stop for an artifact target needing a runtime/ABI change, an identity/banner conflict, a build result that cannot be tied to the accepted T330 source, or a required change outside the named build/status/history surface. |
| Exit Criteria | T330 has a verified `0.5.0330` current developer artifact and immutable record; no historical task target is regenerated; all required checks pass; then T330 can receive its task-level closure summary. |
| Original Owner Request | Fully execute T330 in single-agent mode after its holistic audit of analogous accidental 16/32-bit construction divergence. |
| Similar-Issue Sweep | Inspect current-artifact target, build preset, Status baseline, and T330 history together; classify all 0329 references as retained baseline/history or update the current T330 identity. Do not make a repository-wide historical-artifact rewrite. |

## Current Technical Baseline

- **Current developer artifact:** T330 selects `vm-0-5-0330` /
  `build/output/nxvm_0_5_0330.exe`; commit `af006a19` SHA-256 is
  `F0AEC7780F2BC0A0AEF6451B91CCD4EE25DF6A9D1EE4A5AFA8D1AD3E58F9707A`.
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
| T330 S1 | Accepted progress: the holistic audit converged all 286/386 TSS source/target pairs behind an independent-format private plan, completed the 286 LDTR write/preflight, and corrected 32-bit same-CPL CALL-gate TSS dependence. S3 refreshes the mandatory developer artifact. [History](history/M5-T330-width-path-convergence.md); 211/211 current-gate. |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](history/M5-T329-task-transition-state-machine.md). |
| T328 | Closed the historical LOCK-prefix legality matrix: 8086/80186 retain transparent valid-next-instruction semantics; 80286 adds protected `CPL <= IOPL`; retained 80386 memory-whitelist behavior stays intact. S2 reconciled the current closure map and ordinary matrix, removing the stale Deferred/TODO transfer without changing the user-owned Queue edit. Register, memory, REP, I/O, #GP frame, strict compile, artifact, and 211/211 gate evidence are in [history](history/M5-T328-legacy-lock-legality.md). |
| T327 | Closed the current/specialized-gate reconciliation: fast smoke no longer builds classified media targets, and generated CTest/Ninja evidence now proves the full 210 = 15 media + 195 non-media partition, both developer roots, all specialized verifiers, and the aggregate's two roots. [History](history/M5-T327-current-gate-reconciliation.md). |
| T326 | Closed ordinary protected-mode invalid-opcode delivery: `#UD` now reaches IDT vector 6 with a restartable three-dword no-error-code frame, while the explicit error-code classifier retains vectors 8/10--14/17. Four producer classes, invalid-gate containment, retained VM86 and 210/210 current-gate pass. Real-mode IVT migration transfers. [History](history/M5-T326-protected-invalid-opcode-delivery.md). |
| T325 | Closed the CPU-native Intel 80386DX paging/translation package: CR0, CR2/CR3, 4-KiB PDE/PTE, U/S/R/W, A/D, cross-page atomicity, delivered `#PF`, no-persistent-cache behavior, and pre-486 `INVLPG #UD` are reconciled. Protected `#UD` delivery, task/VM86 paging, and persistent TLB/TR6/TR7 state transfer explicitly. [History](history/M5-T325-80386dx-paging-translation.md). |
| T324 | Closed current-test/specialized-gate separation: `run-current-smokes` now runs only the full 209-test CTest smoke selection, `run-current-fast-smokes` retains the 194-test non-media selection, and `verify-current-specialized-gates` owns 46 named verifiers plus a mechanical target-graph check. `current-gates-gcc` composes exactly both roots; all layer baselines and evidence are retained in [T324 history](history/M5-T324-current-gate-separation.md). |
| T323 | Closed the bounded 80386DX non-task, non-VM86 protection/privilege-transfer composition: direct far transfer, loaded segment rights, 16-bit same/outer gate entry, outer IRET, and parameterized 16-bit call gates now join retained selector, 32-bit, and outer-RETF evidence. The sole S7 serializer correction preflights/copies parameter words. Task/LDT/debug/VM86, paging, legacy LOCK, and x87 retain named boundaries; the 0323 artifact SHA-256 and 209/209 gate result are in the [closure audit](etc/evidence/t323-protection-privilege-closure-audit.md). |

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
