# T560 S1 Complete 486SX CPU Convergence

## Baseline And Claim

Baseline: fb8021291, owner-approved T559 closure. This is the planning
inventory, not CPU qualification. The task must finish all applicable CPU
semantics; transferring an unimplemented row does not satisfy its exit.
Primary-source anchors are retained in the T558 source-availability and
receiver-checklist evidence. Their applicability must be rechecked before
each implementation, especially the selected SX stepping and SL features.

## Coverage Units

Each row covers legal and illegal encodings, operand/address sizes, mode/CPL,
prefixes, flags, fault precedence/restart, reset, actual effects, production
connection and timing wherever applicable. Successful boot or absence of a
TODO is not direct proof. Existing T559 evidence is reused only for unchanged
mechanisms. New tests must exercise real CPU execution, not an independent
reference algorithm standing in for the production path.

| ID | Capability and owner | Current evidence / missing proof |
| --- | --- | --- |
| C01 | Selected silicon identity, CPUID/ID, feature applicability; CPU | T559 candidate 042B/leaf-1 VME is implemented; exact PC110 applicability and feature execution remain open. No production admission based on the candidate alone. |
| C02 | Reset register/hidden state, reset fetch, mode entry; CPU and Core bus binding | cpu.c resets a 486 signature but shares the existing reset body. `_kma_is_reset_vector_fetch` handles 286 and 386 only; 486 provenance is missing. Audit all reset values, debug registers, control flags and cache/TLB invalidation. |
| C03 | CR0/CR2/CR3/CR4, LMSW/SMSW/CLTS and reserved-bit rules; CPU | Existing guest control writes use 386 helpers. Debug CR4 storage is not a guest CR4 contract. Audit all writers and side effects, including task CR3 loads. |
| C04 | Paging WP, U/S, R/W, A/D, PWT/PCD, split accesses; CPU translation owner | `_kma_prepare_physical_linear` only rejects readonly writes inside vpl==3 branches. Supervisor WP is absent. Existing prepare/commit path must remain the sole permission/publication boundary. |
| C05 | Alignment #AC and exception ordering; CPU logical access owner | AC flag storage is not alignment checking. Cover stack, strings, implicit/system accesses, cross-page operands and privilege/mode combinations. |
| C06 | TLB lookup/replacement/invalidation, INVLPG, TR6/TR7; CPU | Current translation walks physical page tables directly; passive TR6/TR7 fields do not implement test commands. Add actual state at the translation owner, including CR3/reset interactions and non-mutating preview. |
| C07 | Internal cache, INVD/WBINVD, TR3/TR4/TR5; CPU and bounded external bus contract | WBINVD remains a #UD TODO; test decoder rejects TR3--5. Need actual tags/data/command effects and external flush/writeback semantics, not success-only handlers. |
| C08 | Descriptor accessed-bit and implicit descriptor writes; CPU | T558 CPU-6 requires already-set versus clear-bit behavior to differ from 386. Full segment/task/gate caller audit still required. |
| C09 | Inherited RMW validation and architectural publication; CPU | Existing High debt includes ALU, shifts, unary, exchange and bit forms. XADD/CMPXCHG fixes do not prove the whole inherited class. Include all such forms used by 486 in this T. |
| C10 | VME, VM86 interrupt/flags virtualization; CPU | Advertised candidate VME lacks operational proof. Audit CLI/STI, PUSHF/POPF, INT/IRET, TSS redirection and pending interrupt behavior against selected sources. Do not infer PVI/PSE from VME. |
| C11 | SMM entry/state save, SMBASE, RSM and exit; CPU plus Core signal/memory binding | RSM remains a #UD TODO. Freeze source applicability, save-map and event priority before implementation; no second executor or board-private CPU state. |
| C12 | IRQ/NMI/SMI, HLT/HOLD, shutdown, reset and power transitions; CPU/Core timeline | Retained family implementation exists but complete 486 transition qualification is absent. CPU owns architectural state; Core owns timeline and board wiring. |
| C13 | Debug registers, traps, RF/TF, exception nesting and restart; CPU | Reconcile inherited behavior with selected family; debugger mutation must not create stale translation/cache state. No new public raw CPU pointer. |
| C14 | SX absent-FPU ESC/WAIT and external error handling; CPU/FPU binding | Preserve no integrated FPU; determine exact SX external behavior from sources. Implementing unrelated x87 arithmetic is not implied. |
| C15 | Full opcode inventory and inherited real/protected/VM86 semantics; CPU | Reuse T559 BSWAP/XADD/CMPXCHG and LOCK proof, but reconcile every decoder/metadata/preview/profile comparison and inherited instruction category. |
| C16 | Complete selected-family timing and penalties; CPU timing owner | `core_machine_cpu_timing_maximum_ticks` has no 486 case; compatibility cost explicitly marks timing unallocated. Need source-form ledger, cache/TLB/bus/alignment/EA/exception penalties, no relabelled 386 timings. |
| C17 | Core production construction, run/reset/debug connection; Core | Production gate still rejects 486. Open only after all applicable preceding rows have proof; board/PC110 firmware qualification remains a separate product task. |

No row is accepted by this initial inventory. Every row needs named sources,
finite subcases and direct tests before completion. Genuine non-applicability
needs hardware evidence; lack of implementation is never non-applicability.

### S2 Base Subcase Evidence

The [base-system record](t560-s2-base-system.md) documents the implementation
and its current qualification. The initial findings above remain the frozen
starting inventory, not a claim that these repairs are still absent.

- C02: documented base reset registers and high reset-fetch provenance now
  have direct proof. Cache/TLB and SMM/power reset effects remain with their
  planned state-owner batches; production admission remains C17.
- C03: 486 CR0 validation/fixed ET and CR3 PWT/PCD normalization, LMSW/CLTS
  preservation and implicit task CR3 publication have direct proof. Real
  cache/TLB effects and exact selected CR4/VME applicability remain open.
- C04: supervisor WP, the full PDE/PTE permission matrix, VM86 access and
  split-page validation/publication have direct proof. TLB/cacheability and
  alignment interactions are not discharged by these tests.
- C08: supervisor-linear GDT/IDT routing, split GDT/LDT descriptors and
  clear/already-set Accessed behavior are implemented at the existing owner.
  The inherited segment/gate/task universe is still subject to C15's full
  selected-family qualification; retained-family regression is not a substitute.

No complete CPU-row acceptance or board-use qualification is inferred from
this bounded batch. Final S2 delivery and acceptance remain in CURRENT.

### S3 Access And Publication Subcase Evidence

The [alignment/RMW record](t560-s3-alignment-rmw.md) reconciles the complete
ordinary operand/RMW batch against original manuals and actual CPU execution.
C05 scalar/stack/string/paired accesses, mode/CPL/AC/AM conditions, implicit
supervisor table exemptions and normal fault delivery have direct proof.
C09 inherited ALU, exchange, unary, bit and shift destination preflight and
publication have direct proof, including cross-page and provider failures.
Zero-count writeback and immediate-fetch precedence are included. The prior
unplanned RMW debt has moved into this implementation/evidence owner.

This does not accept all of C13: same-privilege interrupt/exception-entry
stack alignment and nested fault classification remain part of the already
planned S6 exception-state matrix. C05's whole-row qualification must be
reconciled with that matrix before final CPU admission. Cache/TLB/test
commands, SL/VME/SMM applicability, full opcode and timing qualification
remain open in their planned T560 owners. S3 qualification and acceptance
are recorded in CURRENT and the delivery record, not inferred by this ledger.

### S6 Feature And Exception Subcase Evidence

The [S6 record](t560-s6-feature-exceptions.md#final-source-and-writer-reconciliation)
maps R1--R7 to the complete admitted C01/C10/C13/C14 batch and carried C05
entry/nesting class. C01's candidate VME/FPU feature execution now has direct
proof; exact fitted PC110 applicability is not inferred. C10's CR4, virtual
flags, INT/TSS/IVT, pending flags, IRET and hardware non-redirection use the
single real CPU owners. Valid behavior is source-defined; malformed selected
byte arithmetic and TSS guard inputs retain explicit bounded L2 models.
C13 debug aliases/GD, RF/TF, comparator/shadow, required fetch, exception entry
and provider/nesting/restart cases have direct proof. C05's carried same-level
INT/UD/INTR/NMI stack class is reconciled with segment/page/AC priorities.
C14's absent-FPU ESC/WAIT matrix and actual Core Executor NM delivery have
direct proof, without reopening production construction or duplicating FPU
mode policy in Core. The public neutral FPU fixture remains unchanged.

Final 602/602 units per width, complete specialized gates, eight passing
integration groups and eight verified 0560 EXEs are recorded in the S6
delivery. Coordinator actual-change review accepts pushed 926bfe185; CURRENT
records S6 closure. This evidence does not close C11/C12 SMM/power, C15's whole
inherited corpus, C16 timing or C17 admission; S7--S10 retain them in T560.
No applicable CPU gap is transferred outside this task.

## Initial Direct Inspection

### S4 Translation-State Subcase Evidence

The [S4 record](t560-s4-translation-cache.md) provides direct real-CPU proof
for C06 ordinary/test TLB state, complete INVLPG effective-address/privilege/
failure routes, control/task/reset/debug invalidation and non-mutating preview.
It names the L2 internal replacement and first-dirty-write choices separately
from documented architectural fields. Remaining C04/C16 second-read/locked
page-table and full bus-cycle fidelity must be reconciled by the subsequent
cache/bus and timing batches before complete CPU admission. This obligation
stays in T560 and is not accepted merely because C06 state tests pass.
S4's final qualification, pushed delivery and acceptance are owned by CURRENT
and its batch record. No cache, VME/SMM, whole-opcode or production gate opens.

Read cpu.c reset and debug paths, cpu.h register storage, cpu_instructions.c
physical bus callbacks and translation prepare/commit, and cpu_timing_model.c
profile selector/compatibility fallback. The CPU directory is flat; there is
no separate cpu_memory.c or cpu_control.c. Keep memory and control ownership
at the existing mechanism rather than designing from assumed file names.
The current page walk writes PDE Accessed with a documented 386 rule before
later protection checks; 486 ordering needs source review, not mechanical
reuse or deletion. Cache/TLB state must not make preview mutate the machine.

### Control And Implicit-Access Sweep

- `_s_write_cr0_80386` permits only PE/MP/EM/TS/ET/PG and uses UD for its
  rejected writes. MOV CR on the 486 currently calls this same helper; the
  486 reserved-bit and invalid-combination exception rules require their own
  source-qualified branch at this owner, not a duplicate MOV dispatcher.
- `_s_write_cr3_80386` rejects every non-base bit and only invalidates
  prefetch. 486 PWT/PCD and actual TLB invalidation therefore need work.
- The 32-bit task switch writes `incoming32.cr3` directly. Debug mutation in
  cpu.c also writes candidate CR fields directly. MOV, task load, reset,
  debugger and eventual RSM must share state invalidation ownership while
  preserving their different architectural validation/publication points.
- `_ksa_prepare_code_sreg`, `_ksa_prepare_stack_sreg` and segment-load paths
  set Accessed and require a descriptor write unconditionally. C08 must cover
  already-set descriptors, not only successful clear-to-set cases.
- `_ksa_read_gdt`, `_ksa_write_gdt` and `_ksa_read_idt` use physical-table
  helpers while LDT uses logical accesses. Add explicit GDT/IDT paging and
  supervisor fault tests to C04/C08; do not assume inherited tests prove the
  486 linear-table contract or globally change older-family behavior blindly.
- The public CPU exception vocabulary has no vector-17 AC mask, and its CR0
  bit vocabulary stops at the 386 bits. These are genuine Core CPU contract
  additions needed for C03/C05, not reasons to modify Shared Types.
- The bus provider exposes memory/port/IRQ/FPU contracts, but no cache
  operation or SMI/SMIACT/soft-reset/stop-clock contract. C07/C11/C12 must
  design bounded CPU/Core signals; no App pointer, callback reentry or second
  scheduler is allowed. Existing external-cycle observation is not a cache
  flush command and must not be misrepresented as one.

### Existing Proof Reuse And Its Limits

The current direct 486 references are in cpu_bswap_smoke, cpu_xadd_smoke,
cpu_cmpxchg_smoke, cpu_cpuid_flags_smoke, cpu_legacy_lock_smoke,
cpu_test_register_smoke, cpu_protected_iret_state_smoke and
cpu_task_switch32_state_smoke. Production admission/FPU tests deliberately
reject unqualified 486; protected UD delivery is not positive system proof.
These cover T559's stated delta, not all inherited instructions on 486.

cpu_control_state_smoke currently enumerates 286/386 in its control cases;
cpu_execution_paging_smoke and Core's 80386 paging/descriptor tests are
retained-family regression, not direct 486 WP/AC/TLB proof. Extend existing
owner tests where their contract matches. New cache/SMM state tests need
dedicated owner-local fixtures only when existing fixtures cannot express
the actual hardware contract; no new test framework is planned.

### Source Applicability Recheck

Read-only text inspection of the pinned Intel 272769-004 SX datasheet
confirms SL technology/SMM, SMI instruction/REP boundaries, SRESET retaining
SMBASE, and CPUID VME without integrated FPU for its specified embedded SX.
It explicitly describes 5-V PGA/PQFP packages. This supports the candidate
feature implementation scope but does not prove that PC110's different
package/stepping has identical features. Keep that distinction visible;
do not use SMM absence in current code as evidence that it is inapplicable.
Before numeric constants or save maps are implemented, inspect complete
relevant manual pages and figures, not truncated search excerpts.

## Finite Form Universe And Sources

C15 covers both 256-entry dispatch spaces and every existing group extension,
not only currently reachable BIOS instructions. Classify every encoding as
documented selected-SX, documented unsupported/reserved, or source unresolved.
For documented forms, cover byte/word/dword where applicable, 16/32 code and
address size, register/memory, segment override, REP/REPE/REPNE/LOCK, real,
protected CPL0--3 and VM86. Operand values include carry/overflow/sign/zero,
aliasing, zero/full/oversized shift counts and divide-error boundaries.
Inherited categories are arithmetic/BCD, multiply/divide, shift/rotate/double
shift, bit/scan, move/extend/exchange, stack, near/far control, strings/I/O,
segment/descriptor/task, flags/interrupt/exception, debug/control/test and
absent-FPU ESC/WAIT. New 486 forms retain T559's complete matrices.

C04/C05/C08/C09/C13 add access-kind variants: fetch, explicit data, stack,
string source/destination, page tables, GDT/LDT/IDT, TSS and debugger
observation/mutation. Cross-page and segment-limit cases include failure at
each required fetch/read/write, preserving architectural publication and
documented hardware side effects. Tests must distinguish a correctly
delivered exception from a later shutdown caused by a broken fixture.

C06/C07 add empty/hit/miss/conflict/replacement states, all documented
sets/ways and test-register command fields, cacheable/uncacheable reads and
writes, invalidation with and without a matching entry, reset/control changes
and DMA/snoop visibility. Ordinary paging, cache commands and test-register
instructions operate on the same state, not three disconnected models.

C10--C12 add each documented event from running, halted, shutdown, VM86 and
SMM where applicable, event priority and masking, REP boundaries, locked
accesses, soft versus hard reset and resume. Source-undetermined electrical
details remain explicit before a model is chosen; they cannot become silent
no-ops. Core retains one timeline and one executor.

C16 uses all C15 legal forms plus instruction-fetch, cache/TLB misses,
unaligned/split accesses, EA/prefix, branch, repeat, bus-wait, exception and
interrupt costs. PRM chapter 26 baseline assumptions and Appendix E penalties
are separate inputs. No 386 value or compatibility fallback qualifies a 486
form without evidence; accuracy downgrades still require owner approval.

Primary source map (original hashes and editions in T558 source evidence):

- Intel 1992 PRM chapters 5--6, 9--13 and 21: paging, protection, interrupts,
  debugging, testing, initialization, compatibility and 486 changes;
  chapter 26 and Appendix E: instruction forms and timing/penalties.
- Intel 273021-001 developer manual chapter 8: SMM; chapters 9--10:
  reset/hardware/bus operation; Appendix D: identification and feature scope.
- Intel 272769-004 sections 4.1--4.2 and SX signal descriptions:
  selected candidate identity/features and SL-enhanced SX behavior.

The exact PC110 package/stepping match is unresolved C01 evidence, not a
reason to postpone base-family reset/paging work. Implement source-confirmed
base semantics first. Resolve C01 before claiming silicon-specific feature
qualification or opening production admission. If evidence contradicts the
candidate, revise that single profile's contract explicitly rather than add
an unrequested second CPU variant or suppress a required capability.

## State Ownership And Planned Interface Decisions

CPU owns CR/debug state, translation/cache state and SMM architectural state.
Instruction decode, guest operations, reset and bounded debugger mutation use
that one owner. Preview must neither populate caches/TLB nor set page-table
bits, consume interrupts or write devices. New state is justified by real
hardware-visible operations, not by compatibility flags.

Core owns physical routing and guest-time advancement. A cache-operation
notification and bounded system-signal entry/output may be needed in the
existing CPU/Core interface. Declare exact signal semantics, failure behavior
and consumers in the corresponding implementation brief; no generic event
framework, App dependency or Shared API is planned. Cacheability information
must describe the actual routed memory; do not cache MMIO by default or infer
ROM/RAM from addresses in the CPU. SMM routing must distinguish SMRAM access
without placing PC110 board mapping inside the chip.

C17 includes Core construction validation, machine_scheduler DMA/HOLD exact
286/386 branches, preview/execution timing and board model selection. These
are currently outside the 486 path and cannot be fixed by changing only the
profile upper bound. Existing four products keep their CPU selections.

## Planned Sequential Batches

The following numbers are a plan, not concurrent admission. Each starts only
after the preceding S is accepted; refine before admission if direct evidence
requires a split. No letter suffixes. Estimates are source/test C/H/CMake
added/deleted lines, excluding docs/manifests/artifacts, not quotas.

| Planned S | Complete batch / rows | Source estimate + / - | Test estimate + / - | Exit evidence |
| --- | --- | --- | --- | --- |
| S2 | Base reset, control masks/validation, paging WP and table-address/accessed behavior; C02--04/C08 | 180--400 / 70--200 | 350--650 / 30--100 | Real instructions, reset bus observation, paged GDT/IDT and readonly already-accessed descriptors; old profiles retained. |
| S3 | Alignment and inherited access/commit ordering; C05/C09 | 200--450 / 100--300 | 400--700 / 30--100 | Complete memory-form groups, cross-page preflight, fault priority and restart; no general-read permission hack. |
| S4 | Real TLB and complete INVLPG/TR6/TR7; C06 | 250--500 / 50--150 | 350--600 / 30--80 | Ordinary translation and test commands observe one TLB; MOV/task/debug/reset invalidations and preview proven. |
| S5 | Real cache and INVD/WBINVD/TR3--5; C07 | 400--800 / 40--120 | 450--800 / 20--60 | Read/write/cache attributes, test commands, external operation order and coherence; no fake no-op. |
| S6 | Resolve selected feature evidence, VME and debug/exception/SX-FPU semantics; C01/C10/C13/C14 | 250--550 / 60--180 | 400--700 / 20--80 | Advertised features have direct execution proof; complete mode/privilege/exception matrix. |
| S7 | SMM/RSM and system/power/interrupt signals; C11/C12 | 450--850 / 50--150 | 450--800 / 20--60 | Save/restore, relocation, priority, reset distinction and Core connection; no parallel loop. |
| S8 | Whole inherited opcode semantic matrix and remaining discovered class repairs; C15 | Refine after matrix inventory; no speculative rewrite | 400--900 / 0--100 | Every documented category reconciled, including T559 delta and earlier-family regressions; unresolved forms block closure. |
| S9 | Complete selected timing corpus and dynamic penalties; C16 | 450--900 / 100--250 | 500--1000 / 0--100 | Source-backed form allocation and runtime results; no unallocated selected forms or silent grade downgrade. |
| S10 | Final Core admission and full qualification; C17/all | 30--100 / 20--60 | 150--350 / 20--50 | All applicable rows accepted, real Core reset/run/debug, complete dual-width units/integrations and eight current PC artifacts. |

Implementation estimates may change materially where actual state design is
necessary; report the refined diff before editing. The inherited RMW debt is
in S3, not left outside this T. Unrelated broad x87, assembler or other App
work is excluded because it does not implement the selected SX CPU.

## Validation And Efficiency

During implementation run the cohesive changed-family tests on the original
two persistent Ninja/ccache graphs. Once the batch is stable, run the required
complete repository units once per width and rebuild the eight affected PC
EXEs once, preserving INIs/media and MyNES. Do not rebuild unchanged shared
objects per App. Run relevant external integrations when the changed route
requires them and all external groups at T closure. Passed evidence is reusable
only while its executable inputs and tested contract remain unchanged.

S1 itself changes no executable input: retain the verified 0559 artifacts and
T559 S7 runtime evidence; only governance, link/scope and actual-doc review are
applicable to this planning delivery. No source/test manifest changes occur.

## S1 Delivery Boundary

Planning now covers C01--C17, T558 CPU-2--9, T559 I4--I11 transfers and the
inherited RMW debt with explicit owners, form dimensions and completion
predicates. All implementation rows remain open. The inventory does not claim
to have proved every inherited instruction or settled every silicon-specific
constant; those are the named implementation/qualification obligations above.
The whole task cannot close with an applicable row merely transferred or
untested. No production, test, Shared-six, MyNES, firmware or asset was edited.
