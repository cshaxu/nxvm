# Project Status

## Current Work

**M5 T337 S2 - active.** Reconcile the remaining shared debug, breakpoint,
TF, IRQ/NMI, producer-frame, and reset/transfer rows, including every
current-gate owner made stale by real-mode `#UD` vector-6 delivery, without
duplicating profile-specific instruction work.

## M5 T337 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | The owner approved on 2026-08-13 the Queue-ordered single-agent four-profile program. T337 S1 is accepted in `742c6f01`; S2 is expanded after its complete gate exposed 71 current-gate owner smokes with stale real-mode `#UD` terminal-fault expectations. This bounded reconciliation completes the remaining Shared CPU State And Delivery Foundations proposal rows before profile-specific packages proceed. |
| Objective | Build one exact source-and-evidence disposition for shared debug/breakpoint, TF, NMI, PIC IRQ, software interrupt, exception-frame, task debug-trap, and reset/transfer rows; migrate every current-gate owner made stale by S1 to an explicit valid-vector delivery or deliberately invalid-IVT rollback contract; repair a reproduced shared construction defect only if the route and all affected callers are within this packet. |
| Non-goals | No DR6/DR7 hardware-breakpoint implementation, generic PIC/NMI redesign, reset/shutdown policy, VME/PVI, task/descriptor breadth, profile-specific opcode semantics, x87 execution, or a new public interface. This packet changes owner evidence, not the admitted profile instruction matrix. |
| Reference Baseline | `742c6f01` (`M5 T337 S1 P2: accept real invalid opcode delivery`), the retained `0.5.0335` developer artifact; T321 S2--S4, T316 S50/S60, T329 S7, and T326 are retained evidence inputs. |
| Candidate Proposal | [Shared CPU state and delivery foundations](../proposals/m5-cpu-shared-state-delivery.md); [M5 80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md). |
| Files And ABI Surface | Expected: one indexed T337 S2 evidence record, T337 history, Current, Queue/proposal only if a verified transfer needs correction, one owner-local test support helper/static inventory verifier, and every affected current-gate smoke. Source/CMake changes remain limited to a reproduced shared defect or mechanical owner-evidence enforcement. No public ABI, firmware, or device model change. |
| Applicable Rules | `docs/rules/EXECUTION.md`: mechanism owner, full-P delivery, similar-issue sweep. `docs/rules/ARCHITECTURE.md`: one delivery owner and explicit transition boundary. `docs/rules/CODING.md`: retain owner-local test evidence. `docs/etc/operations/policy/source-policy.md`: Intel authority only; no reference import. |
| Verification | Inspect `ExecInt`, `ExecFinal`, `_e_int3`, `_e_int_n`, `_e_intr_n`, `_e_except_n`, real/protected serializers, task debug-trap post-commit, and every current owner smoke. Record real/protected/VM86 frame width, saved-IP rule, IF/TF effect, NMI-versus-IRQ-versus-TF priority, software versus external origin, and task post-commit distinction. Build a fixed affected-owner inventory from the full current-gate failure and require each entry to declare a valid vector-6 delivery or invalid-IVT rollback disposition. Run all retained focused owners plus current-gate, documentation governance, and diff check. |
| Expected Markers | A durable shared-delivery matrix identifies one owner and proof for every shared row; a mechanical affected-owner inventory prevents a current-gate `#UD` owner from silently retaining the obsolete terminal contract; each ordinary debug/breakpoint, reset/shutdown, and profile-specific residual has exactly one 80386DX Queue or TODO transfer. |
| Asset Needs | Intel 80386 PRM only. No guest media, firmware, external source, binary, or trace. |
| Reporting Requirements | Report a material contract objection before source change. Otherwise deliver one complete pushed P with the source/caller sweep, complete affected-owner inventory and per-owner disposition, retained-owner results, exact transfers, any shared repair and its regressions, and no claim beyond the audited shared boundary. |
| Stop Conditions | Stop if a correct answer needs a DR6/DR7 hardware-breakpoint model, reset/shutdown policy, PIC/NMI device change, VME/PVI, task/descriptor redesign, or broad profile-specific instruction behavior; transfer it rather than widening this audit. |
| Exit Criteria | Every remaining shared-delivery row is proven by a current owner, repaired with a caller sweep, or transferred exactly once to 80386DX/another named later owner; every affected current-gate real-mode `#UD` owner has an explicit vector-6 or invalid-IVT disposition; no duplicate or contradictory delivery claim remains; required gates pass and the P is committed and pushed. |
| Original Owner Request | Implement the Queue in order, in single-agent mode and with a holistic view, through four-profile cross-closure verification. |
| Similar-Issue Sweep | Search all tracked production/test/build/evidence paths for `ExecInt`, `ExecFinal`, `_e_int3`, `_e_int_n`, `_e_intr_n`, `_e_except_n`, `VCPUINS_EXCEPT_UD`, `VCPUINS_EXCEPT_DB`, `flagNMI`, `flagMaskNMI`, `VCPU_EFLAGS_TF`, `dr6`, `dr7`, reset requests, and all vector-1/2/3/6 owner fixtures. |

## Current Technical Baseline

- **Current developer artifact:** T337 S2 selects `vm-0-5-0337` /
  `build/output/nxvm_0_5_0337.exe`; the verified developer output SHA-256 is
  `342FFC183170169729B5BDEE8F4F3512EB86FD27B4EF979D875466927B75E4F0`.
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
| T337 S1 | Accepted real-mode vector-6 `#UD` final delivery: all primary, `0F`, operand, profile, and LOCK representatives converge at `ExecFinal`; valid IVT proof covers the 16-bit restart frame, IF/TF and handler progress, while invalid-IVT rollback retains the terminal diagnostic. A configure-time 69-owner upper-bound inventory prevents a current-gate `tests/machine` `#UD` source from bypassing classification. Focused and 217/217 direct current-gate tests passed. [Evidence](../etc/evidence/t337-s1-real-ud-delivery.md). |
| T336 | Closed the four-profile CPU coverage audit: the primary/`0F` metadata and dispatch forms, real/protected/VM86 state rows, historic evidence, explicit 80386 exclusions, and open CPU debt now have a single indexed disposition. Every residual has one Queue, TODO, or external owner; the next candidate is shared state and delivery foundations. [History](../history/M5-T336-four-profile-coverage.md). |
| T335 | Closed the xasm public-capacity/failure contract: the product facade now uses byte spans, explicit capacities, `type_status`, and commit-on-success result publication; debugger, firmware, and VM debug callback callers migrated with no raw bypass. A dedicated allocation-injection smoke proves capacity, malformed-input, facade-allocation, and engine-allocation failures preserve caller sentinels; `current-gate` passed 216/216. [History](../history/M5-T335-xasm-capacity-failure-contract.md). |
| T334 | Closed EGA sequencer registration atomicity: memory owns coupled provider/observer validation and publication; VADP allocates before publication; allocation and either registry-capacity failure preserve state, while retry publishes exactly one of each. Strict focused smoke and 215/215 current-gate passed. [History](../history/M5-T334-ega-registration-transaction.md). |
| T333 | Closed retained interactive-input failure handling: all 44 Console/debugger readers are inventoried; 43 debugger calls converge at one private boundary; Console stops before parse/execute; EOF/allocation failure and context reuse are covered by two owner-separated smoke targets. Corrective S4 restored its required `0.5.0333` artifact and identity. Evidence and review are in [history](../history/M5-T333-interactive-input-failure-contract.md). |
| T332 | Closed VM session construction drift: one private profile materialization/override path; one early-storage rollback owner with stage-failure and late-media recovery proof; and a fixed 47-owner CPU smoke lifecycle closure with inherited-source migration and static guard. The retained artifact, full gates, and residual historical-fixture transfer are in [history](../history/M5-T332-vm-session-construction-transaction.md). |
| T331 | Closed the bounded real-mode `ExecFinal` final-delivery construction: `#DE/#MF/#BR/#NM/#GP` share one rollback/diagnostic plan, and `#GP` IVT success no longer records a terminal fault before transfer. Real-mode `#PF` is architecturally outside paging's protected-mode state. The new `#GP` frame/failed-IVT regression, mechanical construction verifier, artifact, and 212/212 current-gate result are retained in [history](../history/M5-T331-exception-final-delivery.md). |
| T330 | Closed the whole-codebase construction-drift package: one task-transition constructor, FDD/HDD backing/create atomicity, and CALL-gate dual-fault preflight convergence. Its mechanism matrices remain retained evidence; they are not a current mandatory rule. Intel-required layouts remain explicit; T330 artifact, focused regressions, 211/211 current-gate, and governance evidence are in [history](../history/M5-T330-width-path-convergence.md). |
| T329 | Closed the bounded Intel 80286/80386 protected task-transition state machine: 16/32-bit direct and task-gate entry, nested CALL/IRET state, incoming LDT images, source-CR3 preflight/incoming-CR3 commit, and TSS post-switch `#DB`. S7 proves target-page fetch, target-TSS `#PF` atomicity, and a target-state restart frame; the 0329 artifact and 211/211 gate result are in [history](../history/M5-T329-task-transition-state-machine.md). |

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
