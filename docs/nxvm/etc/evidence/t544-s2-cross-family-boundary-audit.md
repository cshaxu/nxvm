# T544 S2 Cross-Family Boundary Audit

## Scope And Status

Read-only audit against 7393eabaf. This is the S2 audit delivery, not whole-CPU
qualification or implementation of its findings. Shared source/tests and product inputs
are unchanged. Current owns the active packet; the task ledger owns closure.

The original 80386DX Programmer's Reference (1990) was found in the external
manuals-nxvm/cpu archive, SHA-256
9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1.
The historical source identity matches; the archive directory/filename has
changed. PDF pages 101, 207, 208, 233 and 283-285 were rendered and read,
not accepted from OCR alone. They cover printed 3-41, 9-3, 9-4, 10-3 and
14-5 through 14-7.

The 80286/80287 Programmer's Reference (1987) identity is
AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460.
Rendered PDF pages 172, 184 and 294 cover printed 9-10, 10-6 and B-86.
386 PDF page 220 (printed 9-16) was also rendered for the fault-class table.
386 PDF page 265 (printed 12-7) was rendered for RF fault-frame semantics.
The 1981 8086/8088 manual's PDF page 119 (printed 2-100) was rendered for
Flag-Images portability guidance; its original identity remains the S1 source
record's 3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5.
286 PDF pages 328-329 (printed C-2/C-3) were rendered for length, priority,
NMI and early reset differences. 8086/8088 PDF pages 43 and 45 (printed
2-24/2-26) were rendered for inhibition, interrupt priority and return images.
The 1985 family manual's PDF page 247 (printed 2-81), Table 2-30, was
rendered for 80186 reset. Its original SHA-256 is
2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF.

## Confirmed Current-Code Discrepancies

### B01: 8086 Divide-Error Return Address

The manual's real-address compatibility discussion, printed 14-5 through
14-6, distinguishes the failed-instruction return address on 386 from the
next-instruction return address on 8086. Page 284 explicitly supplies the
8086 distinction; the preceding page supplies its 386 context.

Current chain: INS_F6/INS_F7 -> _a_div/_a_idiv -> exception DE -> ExecFinal
-> _e_final_deliver_real_exception -> _e_except_n -> _ser_int_real.
ExecFinal selects instruction_state.data.oldcpu; the delivery helper restores
that whole CPU, and _ser_int_real pushes that restored IP. There is no
generation-specific selection between old IP and decoded next IP. Thus the
8086 frame uses the failed-instruction address instead of the documented next
address. This is a function/state defect, not a timing-tier downgrade.

Repair boundary for owner review: choose the architectural return IP at the
CPU exception-production/final-delivery seam, preserving the single IVT
delivery and failed-delivery rollback path. Preserve old registers and
diagnostic fault origin separately from the delivery frame's return address.
Do not fix BIOS, board, VM or individual DIV call sites. The implementation
must first reconcile the 8088/80186/80286 sources and every DE producer,
including AAM, rather than apply an unproved vector-0 blanket rule.

Required regression: all five CPU profiles; DIV/IDIV register/memory forms,
zero divisor and overflow, prefix/instruction lengths, exact stack return IP,
handler delivery failure, and no successful retirement/time publication.
Existing machine_legacy_timing_normalization_s2_smoke checks 80186 fault
nonpublication and handler entry, not this five-family stack-frame distinction.
Green catalog timing cases do not establish fault return-address correctness.

### B02: 80386 Reset CS Limit

Manual Table 10-1, printed 10-3, note 2 specifies reset CS base FFFF0000h
and limit FFFFh. core_machine_cpu_state_reset sets the correct family base
but assigns cpu_state.data.cs.limit = LIB_UINT32_MAX at cpu.c:531.
That cached descriptor field therefore disagrees with the original manual.
This proves the state mismatch, not every possible out-of-range fetch symptom.

Repair boundary for owner review: restore the documented limit in the sole
CPU reset owner; do not introduce a board reset override. Reconcile the other
four families before changing their shared assignment. Extend the existing
cpu_execution_lifecycle_smoke reset matrix to assert cached limit and retain
its base/first-fetch assertions. Its current cpu_execution_context_reset_case
checks selector, EIP, base, first fetch and 386 EDX, but not CS limit.

The 286 manual, printed 10-6, also explicitly gives CS limit FFFFh.
The same shared assignment therefore disagrees with both protected-era
families' reset cache. That page additionally gives reset MSW=FFF0h:
CPU reset zeroes the entire t_cpu, and SMSW_RM16 directly exposes the low
word of cr0, so the observable 286 reset MSW is currently zero. The repair
must distinguish the reserved/readout bits of 286 MSW from the defined
control fields rather than copy 386 CR0 semantics or add a board override.
Both cold reset and processor-only reset call this same chip reset owner;
Core resolves DEFAULT to 80386 before creating/binding the CPU instance.
Other reset details remain subject to their source-defined/undefined status.

### B04: 8086/8088 Reset Selector And Offset

The 286 manual's compatibility appendix C-3 explicitly contrasts early reset
CS:IP=FFFF:0000 with 286 F000:FFF0. Current core_machine_cpu_state_reset uses
F000:FFF0 for every family; the early reset base is F0000. Both reach physical
FFFF0, so the existing first-fetch check passes, but the visible selector/IP
and CS-relative addresses differ. cpu_execution_lifecycle_smoke explicitly
expects F000:FFF0 across its matrix and therefore encodes the wrong early
state rather than merely omitting the assertion.

Repair for owner review: choose source-defined selector, offset and cache at
the sole CPU reset owner while preserving each documented physical first
fetch. Update both context and opaque-instance reset regressions and test
CS-relative access before the first far transfer; no board alias can correct
the visible registers. The separately rendered 80186 Table 2-30 also specifies
CS=FFFFh and IP=0000h, so its current F000:FFF0 row has the same mismatch.
That table gives status word F002h. Current reset stores 0002h, and the early
FLAGS-image helper removes bits 12-15. The repair must distinguish a valid
canonical writable internal representation from its architectural readout and
saved image; simply inserting F002h into every flags load is not a solution.
DS/ES/SS=0 agrees with that table. Its relocation/UMCS reset values belong to
80186 integrated peripherals, not fields implemented by the current CPU
instruction profile; S4 must inventory that capability boundary explicitly.

### B03: 80386 Interrupt/Debug Boundary Batch

Four current-source discrepancies are directly supported by the original:

- B03a, MOV/POP SS single-step suppression: printed 9-4 requires suppression
  of debug/single-step exceptions after MOV/POP SS until the following
  instruction boundary. MOV_SREG_RM16 and _e_pop_sreg set flagMaskInt, but
  _debug_complete_instruction still schedules TF and _debug_deliver_trap
  delivers it without consulting that inhibit. A TF-enabled successful SS
  load therefore traps at the very boundary which must be suppressed.
- B03b, LSS: printed 3-41 explicitly says interrupts are not inhibited at
  the end of LSS. LSS_R32_M16_32 calls _e_load_far, which sets flagMaskInt
  whenever its target is SREG_STACK. ExecInt consequently delays delivery
  after LSS. LES/LDS/LFS/LGS are the same helper's other callers; none has
  a stack target. Remove the misplaced policy from this shared load helper,
  rather than patch a board or add another LSS implementation.
- B03c, NMI re-entry: printed 9-3 and 14-7 require NMI blocking after
  recognition until IRET. The only flagMaskNMI setter is the external
  core_machine_cpu_set_nmi_mask; request_nmi latches a request when that
  external mask is clear. ExecInt clears the pending request on successful
  delivery, but does not establish internal blocking. IRET and its real,
  same-level, outer-level, VM86 and task-return helpers have no NMI-block
  transition. A new admitted edge can therefore re-enter before IRET.
  External board masking and CPU architectural blocking are different facts;
  do not reuse the externally writable mask for both.
- B03d, priority: printed 14-7 states that 386 single-step takes priority
  over external interrupts. ExecInt currently enters pending NMI first and
  calls _debug_deliver_trap second. That permits a pending TF trap to be
  delivered in the newly entered NMI context instead of preserving the
  required pre-NMI debug context. The INTR branch is already after debug.
  Do not apply the 386 ordering to 8086 without its own documented rule.

These are functional boundary defects, not newly introduced L1 timing labels.
Existing cpu_legacy_sreg_stack_smoke checks POP-SS INTR deferral; it does not
prove TF suppression. cpu_lss_lfs_lgs_smoke covers load/state/fault behavior,
not LSS with pending IRQ/NMI. cpu_execution_signal_prefetch_smoke checks
external mask/HLT wake and cpu_execution_fault_event_smoke checks delivery
success/failure; neither establishes block-until-IRET. cpu_debug_state_smoke
checks MOV-DR/instruction/data breakpoint behavior, not simultaneous SS/TF/NMI.

Coherent repair for owner review: retain one CPU delivery arbiter; distinguish
short INTR inhibition, SS-related debug/NMI inhibition and NMI-in-service
blocking inside the CPU owner. Instruction producers declare their actual
documented effect, not merely that they write SS. Use the profile's documented
priority at the arbiter. No new public API, event queue or VM/BIOS special case
is needed. Before implementation, reconcile expiry, faulted IRET, reset and
each earlier generation. Regression must exercise simultaneous requests,
successful/failed SS loads, LSS versus MOV/POP SS, TF/data/instruction debug
causes, external NMI mask versus internal blocking, failed entry and IRET.

The rendered 286 appendix C-2/C-3 extends B03c/B03d to 286: single-step
precedes external interrupts, and recognized NMI blocks further NMI until
IRET. The shared arbiter has neither profile priority nor internal NMI state.
The same fix must cover both generations, while 8086/8088 remain different.

8086/8088 Table 2-3, printed 2-26, orders NMI, INTR, then single-step.
ExecInt instead orders NMI, pending debug trap, then INTR for every family.
TF capture/scheduling is not restricted to 386, so the early INTR/TF ordering
is also wrong. The coherent arbiter repair must preserve this early ordering,
not universally move Debug ahead of all external requests.

The early manual's printed 2-24 footnote inhibits interrupts after MOV/POP
to a segment register, not only SS; the main text also delays reenabling after
IRET until the following instruction. Current MOV_SREG_RM16/_e_pop_sreg
set the inhibition only for SREG_STACK; IRET has no such declaration. These
are additional early-family contexts in B03's source-qualified producer grid.
The existing POP-SS versus POP-ES/DS IRQ deferral grid is a 386 fixture, not
early-family proof. Do not change that valid later-family expectation.
Printed 2-26 permits NMI interrupting an early interrupt procedure; it does
not justify installing the 286/386 block-until-IRET rule on early CPUs.

### B05a: 80386 Runtime Instruction-Length Enforcement

Printed 14-6 requires a general-protection exception for instructions longer
than 15 bytes; 8086 has no such length limit. The copied lexeme scanner checks
index/length against 15, but ExecIns does not call that scanner: it loops on
prefixes using _s_read_cs, dispatch, _s_test_eip and _s_test_esp. Those runtime
read/skip helpers check segment/offset boundaries, not accumulated instruction
length. The oplen field is initialized to 15 as an observation size, not
decremented as an execution limit. Thus a sequence of 15 valid prefixes plus
NOP is not rejected by the actual 386 executor's length contract.

Repair for owner review: enforce the family instruction-byte limit in the
existing runtime decode/fetch owner, including ModR/M, displacement and
immediates. Do not treat the separate preview scanner as an execution gate
or reuse a 15-byte limit for 8086. Include legal 15-byte and illegal 16-byte
cases, prefix-only input, segment-end/wrap, no body-side effects and exact
exception restart. Existing prefix_attributes_s64 tests valid prefix
combinations/last-wins/repeats/LOCK; they do not cover the overlong boundary.

The rendered 286 appendix C-2 gives a different precise contract: a 10-byte
instruction limit, with exception 6 on violation. The same runtime read/skip
path has no 286 accumulated-byte guard. Its repair therefore needs a
profile-qualified byte limit and exception kind, not a universal 15-byte/GP
condition. Test legal 10-byte and illegal 11-byte 286 forms in addition to the
386 boundary and the early CPUs' lack of this limit. 80186 remains a separate
source-reconciliation row, not an inferred midpoint between these families.

### B05b: 8086 FLAGS Image Evidence Was Too Narrow

Printed 14-7 explicitly describes bits 12-15 as set in the 8086 stack image
from PUSHF/interrupts/exceptions. The 1981 8086/8088 User's Manual's Flag-Images
guidance instead tells software not to rely on undefined image bits and to
mask them. The latter portability advice does not erase the later explicit
compatibility observation. Reconcile both sources, rather than claim neither
contains an observable rule.

_e_real_flags_image_16 currently uses the same defined-bit mask as load and
therefore emits zero in those positions on 8086. Separating load and image
helpers structurally was correct; sharing the writable-defined mask is not
sufficient evidence for the physical image. cpu_pushf_popf_smoke masks the
positions before comparison, so its pass cannot establish the later manual's
observed image behavior. Repair must preserve canonical internal/writable
flags while supplying a source-qualified outgoing image to PUSHF and the
single interrupt/exception frame path. Reconcile 8088/80186/80286 separately;
do not make a new claim about all reserved bits or change timing labels.

### B05c: 80286 POPF Privilege And Real-Mode Preservation

The 286 POPF page B-86 explicitly requires IOPL changes only at CPL0 and
IF changes only when CPL <= IOPL; insufficient privilege preserves those
bits without an exception. In real mode NT and IOPL must not be modified.
Current POPF applies privilege/mode preservation only to profiles >=80386.
Its earlier-family branch pops the word and passes it directly to
_e_eflags_load. The 286 defined mask contains NT/IOPL/IF, and the helper
does not apply CPL or real-mode preservation. Thus both protected privilege
and real-mode rules are absent for 286. This is not a missing feature on
8086/8088/186, which do not have these protected-state fields.

Repair for owner review: let the sole POPF producer select the family's
writable mask from mode/CPL/IOPL, then use the existing canonical load path.
Preserve the 386 RF/VM/operand-size rules, the table-style handler and the
earlier families' behavior. Do not solve this by globally masking NT/IOPL
for all loads: task switches and IRET have different contracts. Extend the
current pushf_test_protected_iopl grid to 286 and add explicit 286 real-mode
preservation; its current protected fixture is 386-only. S5 must additionally
reconcile the independent IRET/task-state load rules.

### B06: Protected Nested-Fault Classification

- 286 page 9-10 requires double fault when another fault occurs while
  delivering the specified first faults (0, 10, 11, 12, 13), and shutdown
  if double-fault delivery itself fails. ExecFinal's nested-fault branch is
  restricted to profile >=80386; 286 can never enter that branch.
- 386 page 9-16 classifies DE as contributory and requires double fault for
  contributory/contributory, PF/contributory and PF/PF pairs. The current
  _e_is_contributory_exception includes only TS/NP/SS/GP, omitting DE;
  ExecFinal tests only the two-contributory condition, omitting both PF-first
  cases. Existing four delivery-failure vectors in
  cpu_execution_fault_event_smoke exercise 386 GP combinations only.

Repair for owner review: one profile-qualified exception-class decision at
the existing delivery owner, covering the complete ordered pair table, then
reuse one delivery/zero-error/shutdown path. Do not merely change >=386 to
>=286: the families' classifications differ. Include successful secondary
delivery, required DF, zero error code, failed DF shutdown, return/rollback
state, CR2 preservation, and no success-time publication. Segment-overrun
producer availability and NMI/RESET recovery from shutdown must be reconciled
explicitly rather than silently treated as covered by this classification fix.

The same rendered 386 page also requires serial handling for the ordered
pairs which are not DF (including contributory followed by PF). ExecFinal
attempts first delivery, but on a non-DF secondary failure it restores the
original exception/code instead of dispatching the secondary exception. The
remaining protected path publishes record_fault and request_stop. There is
no secondary delivery/restart state in this branch. A valid PF handler thus
cannot service the failed first handler's page access through this path.
The batch must cover both serial and DF dispositions, not only enlarge the
contributory predicate. Tests must distinguish first-handler memory faults,
descriptor faults and successful secondary service from rejected DF cases.

Shutdown is another distinct disposition in this same batch. The manual
describes no further instruction execution until NMI or RESET. Current chip
handling emits shutdown_requested plus stop_requested; the generic Core stop
consumer cold-resets the machine. An attachment may explicitly consume
shutdown as a processor reset (the existing DeskPro policy), but that board
choice is not proof of the generic CPU's architectural shutdown/NMI recovery.
There is no retained chip shutdown state in the traced request/consume path.
The repair proposal must separate CPU shutdown, product stop and an explicit
board reset response; preserve the valid board-reset consumer rather than
remove it or make every shutdown an automatic board cold reset. NMI/RESET
recovery and failure/cancellation need CPU-only and Core integration evidence.

### B05d: 80386 RF In Non-Debug Fault Frames

Printed 9-4 and 12-7 require RF in the saved EFLAGS image on fault entry,
including faults other than instruction-breakpoint faults. ExecFinal adds RF
to fault_cpu only for VCPUINS_EXCEPT_DB. For a GP/NP/PF fault with incoming
RF clear, it restores fault_cpu and calls the normal exception delivery path.
_ser_int_protected_32_same copies cpu_state.data.eflags into oldeflags and
pushes that unmodified value; the outer-level helper likewise has no RF
fault-image transformation. Thus an ordinary 32-bit fault gate frame lacks
the documented RF bit. This is a saved-image defect, not evidence that every
live-handler RF value or every task-gate path is incorrect.

Repair for owner review: classify fault versus trap/interrupt at the existing
exception owner and construct the qualified saved FLAGS image once. Preserve
the entry state used for rollback/diagnostics; do not add RF to arbitrary
software INT, data-breakpoint traps or external interrupts. Exercise RF-clear
and RF-set inputs, GP/NP/PF and instruction breakpoint, same/outer privilege,
16/32-bit gate widths, IRET retry and failed frame delivery. Current
cpu_debug_state_smoke tests incoming RF suppressing a breakpoint and clearing
after a successful ordinary instruction; that does not test other fault frames.

Printed 12-7 also exempts IRET, POPF and task-switching JMP/CALL/INT from the
ordinary completion-clearing rule. _debug_complete_instruction exempts only
opcode CF when debug_rf_before is set. Task-switch FLAGS loading and this
completion condition require an explicit S6 reconciliation before changing
them; they are not proven correct by the fault-frame fix.

### B07: Retirement Observation Is Not Physical-Time Publication

The installed-observer contract in retirement_observation_interface.h
explicitly says callbacks precede physical-contract rejection and elapsed-time
publication. The implementation captures copied current/entry CPU state,
source timing and pre-publication elapsed/timeline values, then calls the
observer without advancing time. Therefore the ordering in
core_machine_publish_successful_retirement is consistent with its public
contract; moving the callback after eligibility would change that contract,
not repair a demonstrated premature-clock-publication defect.

The existing retirement_unallocated_profile_case in
machine_retirement_observation_s3_smoke explicitly expects one observation,
zero executed/time counters and unchanged timeline when physical qualification
rejects an unallocated form. This is direct regression coverage for that
distinction. Both immediate and external-cycle-delayed successful completion
use the same publication helper; consumed synchronous fault delivery returns
before timing selection. This accepts only the structural observation/time
boundary. CPU cycle formulas, eligibility-key completeness and all delayed
failure contexts still require their family/source qualification, not an
observer-order patch.

### Family Receiver Lead: 8086 IDIV Minimum Quotient

Printed 14-7 also says 8086 raises divide error for quotients 80h/8000h,
unlike 386. _a_idiv's current byte/word checks accept those signed minima
without a CPU-profile condition. This is a concrete S3 arithmetic lead,
not an accepted all-family diagnosis or a reason to patch individual callers.
S3 must reconcile every signed-division size/error form and generation.

## Remaining Qualification Rows And Receivers

- B03 remainder: STI's exact NMI effect, earlier-generation inhibition/priority,
  80186 rules, shadow expiry and faulted IRET. The 8086/8088 and 286 source
  differences above now join the confirmed grid; STI-specific NMI behavior
  still must not be inferred from the shared mask. The complete B03 producer/
  expiry/priority repair batch owns this proof before implementation; S3/S4
  reconcile early/186 sources and S5/S6 the protected families.
- B04 remainder: reset fields outside the directly proven CS/MSW values,
  source-defined versus undefined state, and all CPU-only reset regressions.
  DEFAULT resolution and the two production reset callers are traced above.
  B02/B04's reset batch owns source-defined readout regressions; S3-S6
  reconcile unspecified family fields, not invented reset constants.
- B05 remainder: full metadata/admission and FLAGS privilege/load callers.
  The specific length/image discrepancies above do not qualify all forms.
  S3-S6 own complete legal/illegal form and mode/privilege expansion, with
  B05's length/image/POPF/RF batches carrying the confirmed shared defects.
- B06 remainder: exact serial secondary restart/error-code state, NMI/IRET and
  shutdown recovery regressions, including task/gate transitions. The traced
  serial-stop and generic shutdown-reset gaps are recorded above; fixing the
  pair classifier alone cannot close this batch. B01/B06's complete delivery
  batch owns these contexts; family S3-S6 retain source qualification.
- B07 remainder: complete delayed failure/cancellation and eligibility-key
  contexts. The observation/publication ordering is reconciled above, not a
  defect; that bounded conclusion does not qualify the physical time axis.
  S3-S6 own source timing/form eligibility and affected delayed-failure proof.

These qualification rows remain pending in T544, with their complete batch
receivers named above. S2 identifies their boundaries and missing proofs; it
does not silently accept them or move them outside this T. No new L1, new
timing claim or Shared patch is asserted by this audit delivery. Coordinator
acceptance and S closure are separate from this executor record.

## Fresh S2 Verification

Complete repository-only units ran once per width against the unchanged
7393eabaf runnable baseline: x64 506/506 in 60.41 seconds; x86 506/506 in
60.39 seconds. Both used RunTestAggregate.ps1 with four jobs and a 300-second
deadline, sequentially, in the retained default receiving caches. Both process
handles completed with exit zero; no owned test process remains active.
These passes do not cover the missing boundary regressions identified above.
No production/test/build/asset input changed, so no EXE rebuild is required.

## Structural Boundary Inventory Reconciliation

These bounded traces do not accept the complete instruction families. They
identify the actual mechanism and the exact receiver of remaining semantics:

| Boundary | Inspected current path | Disposition and remaining receiver |
| --- | --- | --- |
| CPU identity | Core resolves DEFAULT; chip instance binds one retained profile | Sentinel is not a CPU; five-family semantic contexts remain T544. |
| Reset entry | Both cold/processor reset call chip state reset; context and opaque-instance regression share the same expected matrix | B02/B04 require one profile-qualified reset repair; first-fetch equality alone is insufficient. |
| Form admission | metadata_get primary/0F/ESC cases; profile_allows_form; primary ExecIns and 0F dispatch callers | One shared metadata gate exists. It only proves minimum-CPU admission, not mode/privilege/ModR/M legality. Full legal/illegal expansions remain S3-S6; length gaps are B05a. |
| FLAGS outgoing image | PUSHF word branches and sole real interrupt frame call image_16; protected 32-bit frame captures EFLAGS | B05b/B05d; one producer cannot use a writable mask as a universal saved-image rule. |
| FLAGS incoming canonicalization | Task loads (32/16/backlink), IRET same/outer/VM86/real, POPF mode branches call eflags_load | B05c is confirmed POPF defect. IRET/task checks remain S5/S6 and must not inherit POPF-specific permissions globally. |
| Boundary arbitration | ExecInit clears short inhibit; SS/STI producers declare it; ExecInt orders NMI/Debug/INTR; IRET has no NMI state | B03 batch requires a full profile-qualified producer/expiry/arbitration grid; 80186-specific uncertain contexts remain explicit source proof, not inferred behavior. |
| Fault completion | ExecFinal restores old state, selects first exception, attempts entry, handles limited DF or stop | B01/B06 cover restart, ordered secondary exceptions and shutdown; no successful instruction time may be inferred from handler entry. |
| Immediate retirement | Fault delivery is consumed before timing selection; otherwise eligibility/observer helper precedes executed/ticks publication | Observation ordering is intentional; family timing/form eligibility remains S3-S6. |
| Delayed retirement | External-ready wait and remaining waits publish external-wait time; completion uses the same retirement helper | Wait ticks are not retired-instruction ticks. Budget exits retain the pending completion; cold/processor reset clear it. Source eligibility and affected failure/cancellation regressions remain in family/repair batches, not accepted by structural trace alone. |

The trace used rg over metadata, FLAGS image/load, inhibit/debug pending and
retirement-wait/publication symbols, followed by the enclosing function bodies
and the exact reset/observer regression assertions. No generic device framework,
second decoder or observer-as-clock implementation is proposed. Every
structural audit row has a code path, observed disposition and remaining
qualification receiver. This exhausts S2's boundary-inventory objective, not
the family qualification or repair objectives.

Delayed-path review additionally inspected retirement_wait_contract in
machine_prefetch_locality_smoke: budget retention, bus-not-ready wait, ready
completion and rejection before unqualified physical wait time are asserted.
Cold reset clears the observation, CPU and delayed completion; processor reset
clears CPU instruction/debug state and delayed completion while retaining board
time. Execution_refresh runs ExecIns then ExecInt, except debug pause; short
inhibition is cleared by the next ExecInit, not a wall-clock timer. These facts
define the existing lifecycle boundaries, not proof that the missing B03/B06
rules work. Failed IRET and family-specific inhibition need the exact source
and regressions in those complete repair batches.

## Minimal Coherent Repair Proposals For Owner Review

No Shared implementation is admitted by this report. Four mechanisms cover
the findings without new public APIs or per-board CPU workarounds:

1. Reset/readout: cpu.c sole reset owner selects documented family state;
   cpu_instructions.c keeps FLAGS writable state distinct from saved image and
   286 MSW readout. Extend the existing five-family lifecycle/FLAGS fixtures,
   preserving physical first fetch and undefined-state distinctions.
2. Decode/admission: existing runtime fetch/decode counts complete instruction
   bytes, with source-qualified limit/exception per family. Preserve the table
   handlers and one metadata authority; do not make preview scanning a second
   executor or impose 386 rules on early CPUs.
3. Boundary arbitration: instruction producers declare only documented inhibit
   effects; one CPU-owned arbiter selects family priority, tracks short shadow
   separately from NMI-in-service and external mask, and handles IRET/reset.
   Complete the early/186 source grid before implementing uncertain rows.
4. Exception delivery: select return IP and fault/trap FLAGS image once; one
   family-qualified ordered-pair classifier selects serial delivery, DF or
   shutdown, preserving one entry/preflight/rollback path. Keep explicit board
   reset response separate from CPU shutdown and product stop. Include all
   affected same/outer/task/real/VM86 and failure contexts, not GP-only tests.

Expected production surface is cpu.c, cpu_instructions.c, CPU-private state
headers where actual state is needed, and x86/core/machine.c only for the
shutdown-versus-stop boundary. Matching CPU/Core tests extend existing owner
fixtures; manifests update only after an approved Shared edit. No Lib/Common,
MyNES, INI, ROM, timing-tier downgrade or handler-style rewrite is proposed.
Actual runnable changes require all affected PC x64/x86 artifacts, complete
units and affected integration under the normal packet; this audit changes
none of those executable inputs.
