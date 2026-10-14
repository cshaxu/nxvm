# T559 S1 Instruction Inventory

## Status And Baseline

S1 planning delivery, pending coordinator actual-change review. This is not
runtime qualification; every implementation row below is still unimplemented.
Inspected baseline: `29ba073f39cbf68d6ef662e3a4f3215ddbe05027`.
The active packet and [proposal](../../proposals/m5-pc110-486-instructions.md)
own scope. Source editions, hashes and CPU-1--7 contracts remain in
[T558 sources](t558-s2-source-availability.md) and
[receiver checklists](t558-s2-receiver-checklists.md).

This record starts from actual Core code. No Shared-six changes, external
source imports, production changes or executable qualification accompany it.

## Confirmed Current Owners And Gaps

All source paths below are beneath `src/core/chips/cpu/`.

| Mechanism | Current code evidence | Required disposition |
| --- | --- | --- |
| CPU identity | `cpu_interface.h` profile enum ends at 80386; `cpu.c` profile names, reset base and reset EDX selection have no selected 486 entry | Introduce an explicit selected identity only after optional-feature decisions; retain existing values and default 386 |
| Decode admission | `cpu_instructions.c`, `core_machine_cpu_instruction_metadata_get()` rejects new 0F forms independently of the dispatch table | Update metadata and execution together; a handler alone is insufficient |
| Instruction length | `core_machine_cpu_instruction_lexeme_scan_with_options()` rejects profiles above 80386 and permits 0F scanning only for 286/386 | Extend the existing scanner, including ModRM/no-ModRM classification; no second decoder |
| BSWAP | Dispatch 0F C8--CF is `UndefinedOpcode` | Eight 32-bit register encodings, flags unchanged; explicitly resolve undefined 16-bit behavior without calling it documented #UD |
| XADD | Dispatch 0F C0/C1 is `UndefinedOpcode` | Byte/word/dword, register and memory, address-size and LOCK forms; same-register alias and failed-store atomicity |
| CMPXCHG | Dispatch 0F B0/B1 is `UndefinedOpcode` | Byte/word/dword, equal/unequal, accumulator/destination/source aliases, memory permission and failed-comparison write behavior |
| INVD/WBINVD | 0F08 is undefined; `WBINVD()` is a TODO that raises #UD; metadata rejects both | Instruction legality/privilege here; real cache effects belong to the system receiver, never a success-only placeholder |
| INVLPG | Metadata admits 0F01 only through /6; no INVLPG handler | /7 memory-only effective-address semantics and privilege here; invalidation semantics have a named system receiver |
| Test registers | `_d_modrm_treg()` rejects TR0--5, selects existing TR6/7 fields; MOV handlers read/write those fields | Audit 486 TR3--7 and test-command effects separately from retained 386 TR6/7; adding fields alone is not complete behavior |
| CPUID | `CPUID()` is a TODO raising #UD and 0FA2 has no metadata admission | Selected-stepping identity, leaves and ID-bit rules; no unconditional generic-486 feature claim |
| RSM | `RSM()` is a TODO raising #UD | SMM/system receiver; not part of ordinary integer implementation |
| CR writes | `MOV_CR_R32()` calls `_s_write_cr0_80386()` / `_s_write_cr3_80386()`; only CR0/2/3 accepted | System receiver must own 486 control effects and any CR4/VME extension; do not mechanically rename or broaden |
| FPU admission | `core_machine_cpu_allows_fpu()` explicitly binds 287/387 to existing profiles | SX does not gain an integrated FPU merely by using a higher profile enum; preserve ESC/WAIT exception distinctions |

## Required Coverage Dimensions

Each implemented instruction form needs direct CPU-owner evidence, not only a
BIOS checkpoint or a metadata test:

- decode/length agrees with execution, including truncated instruction fetch;
- old profiles reject new opcodes without operand side effects;
- legal operand/address widths and prefixes, including illegal LOCK forms;
- register aliases, flags, and untouched-register preservation;
- real/protected/virtual-8086 privilege where applicable;
- segment/page/read/write faults and restart state before committing results;
- nominal instruction timing source and explicit system-level timing limits.

Existing reusable owner fixtures are in `test/core/chips/cpu/support/`.
`cpu_decode_admission_smoke.c` already distinguishes instruction fetch from
operand access and injects failed suffix fetches, but its profile loop ends at
80386. `cpu_control_state_smoke.c`, `cpu_pushf_popf_smoke.c`,
`cpu_iret_state_smoke.c`, `cpu_eflags_local_smoke.c` and profile inventory runners
are audit entry points, not evidence that the new profile is covered.

## Decode And Inherited-State Reconciliation

The scanner's ModRM classification already includes CMPXCHG B0/B1 but not
XADD C0/C1. Its form validator separately rejects 0F01 /7 and TR indices below
6. Both decisions therefore need profile-aware treatment; extending metadata
alone would leave valid 486 forms unscannable. BSWAP and CPUID have no ModRM.

`PREFIX_LOCK()` owns actual execution admission. Its 0F switch accepts only
BTS/BTR/BTC and the immediate bit group, not XADD/CMPXCHG. `_d_modrm()` rejects
a locked register operand. The length preview separately rejects every LOCK
prefix: preview unavailability is not itself a guest #UD. Preserve that
distinction when testing or changing it; do not infer runtime illegality from
a failed preview. The existing primary LOCK switch also groups ordinary
two-operand ALU/XCHG opcodes with FE's `/0..1` restriction. This is a concrete
inherited candidate defect to reconcile against the retained-family contract,
not permission to blindly broaden every opcode in the switch.

`cpu.h` has active `VCPU_EFLAGS_RESERVED = 0xfffc802a`; AC/VIF/VIP/ID definitions
are inside a comment. The active mask is consumed by task loading, IRET,
PUSHF and POPF paths, so merely enabling the ID constant cannot implement
ID toggling. Introduce one profile-owned writable/reserved-mask decision at
the existing flags boundary, with privilege masks still owned by the relevant
instruction. Keep VIF/VIP and alignment effects with the system receiver.
`_e_exception_vector()` currently stops at vector 16 and has no #AC delivery;
changing the AC bit does not establish alignment-check support.

Exact 80386 comparisons outside timing have these dispositions:

- `cpu.c`: reset base/name needs the selected identity; reset signature needs
  the frozen feature decision, not a blind 386-to-486 name substitution.
- `_kma_is_reset_vector_fetch()`: selected 486 reset provenance is a system
  handoff; retain 286's distinct physical bus width.
- Scanner bounds and extended-opcode eligibility: instruction receiver.
- `core_machine_cpu_allows_fpu()`: retain explicit 287/387 eligibility, do not
  extend it numerically to SX.
- `LAR/LSL` area's exact comparison belongs to LSL source-timing observation,
  not functional descriptor-limit calculation. Retain the 386-only observation.
- `cpu_timing.c` and `cpu_timing_model.c`: source-table selection, formula
  operands, source keys and maximum budgets belong to the system/timing
  receiver. Never extend 386 timing eligibility just to make 486 execute.

The existing `core_machine_valid_cpu_profile()` in `src/core/x86/machine.c`
rejects profiles above 80386. This is the concrete production admission gate
to retain during instruction work, not a new persistent eligibility flag.
CPU-owner tests can bind the selected profile through the existing execution
context without making a PC App runnable with an unfinished 486. Scheduler
286/386-specific branches remain system-receiver work before that gate opens.

## Hardware-State Boundary

The original PRM sections 10.6.2--3 and 10.7.2--3 identify real effects:
TR6 writes trigger TLB lookup/write commands; TR3 transfers cache fill/read
buffer words selected by TR5; TR5 triggers cache commands. These are not
passive architectural scratch registers. Keep their legal instruction forms
in this task's ledger, but require the system receiver to supply the actual
state operations before normal execution can succeed. Preserve existing 386
TR6/TR7 behavior as retained baseline, without claiming it proves 486 tests.

Selected CPUID remains source-qualified: the recorded PC110 identity 042B
does not include a complete feature capture. The later SX datasheet specifies
VME, which current Core does not implement. Do not solve this by inventing a
zero-feature 042B CPU or advertising VME in an admitted runnable product.
The instruction tests may establish leaf/ID encoding for the explicitly named
candidate contract; production admission remains closed until the system
receiver reconciles silicon applicability and implements advertised effects.

## Proposed Cohesive Batches

This is the numbered sequence for coordinator review at S1 exit, not
admission of concurrent work:

| S | Complete batch | Exit proof |
| --- | --- | --- |
| S2 | Selected CPU identity, decode infrastructure and BSWAP | New profile has no production-board admission; eight registers, widths/prefixes, flags, old-profile rejection and scanner/execute agreement |
| S3 | XADD | All widths and register/memory forms, aliases, flags, LOCK, address sizes, fetch/store failures and EM/TS independence |
| S4 | CMPXCHG | Equal/unequal paths, all widths, accumulator aliases, permissions, memory-write semantics, faults and LOCK |
| S5 | INVD/WBINVD/INVLPG and test-register instruction boundary | Legal/illegal encodings, CPL/v86 rules and effective-address behavior; hardware-state completion explicitly owned, no success-only cache/TLB stub |
| S6 | Selected CPUID/ID and inherited flags writes | Source-qualified leaves/ID, reset candidate separation, POPF/IRET/task flag masks, old-family preservation; no inferred VME implementation |
| S7 | Whole instruction-delta regression and handoff | Every frozen form reconciled, retained profiles and negative paths verified, system receiver accepts all remaining non-runnable obligations, required build/test/artifact gates |

Each implementation packet will name its exact source/test files and estimated
diff before code changes. Batches may split only on evidence and use subsequent
linear S numbers; the table is not authority to skip an unresolved prerequisite.

## Frozen Form Ledger And Completion Predicate

Coverage unit is an encoding family crossed with the dimensions below, not
one successful retirement. `Pending` requires implementation and direct proof;
`system gate` is an explicit non-runnable dependency, not accepted semantics.

| Row | Finite forms and additional proof | Initial disposition / receiver |
| --- | --- | --- |
| I1 | 0FC8--CF, all eight GPRs, 32-bit result, flags, 16-bit undefined-form policy, 66/67/F0 and truncated fetch | Pending / S2 |
| I2 | 0FC0 byte and 0FC1 word/dword; all GPR encodings including high bytes; register/memory, same-register and overlapping byte aliases, ADD flags and source exchange, 16/32 addressing, LOCK-memory versus LOCK-register, EM/TS combinations | Pending / S3 |
| I3 | 0FB0 byte and 0FB1 word/dword; equal and unequal accumulator compare, source/destination/accumulator aliases including high bytes, subtraction flags, write permission and failed-store restart, 16/32 addressing and LOCK | Pending / S4 |
| I4 | 0F08 and 0F09; real/CPL0, CPL1--3 and v86; invalid LOCK, fetch failure; no false successful cache operation | Pending instruction contract / S5; cache effects system gate |
| I5 | 0F01 /7, memory and rejected register forms; 16/32 effective address, segment/privilege ordering, no ordinary target data load, LOCK rejection | Pending instruction contract / S5; actual TLB invalidation system gate |
| I6 | 0F24/26 both directions, all ModRM reg indices 0--7, memory-form rejection, CPL0/nonzero/v86, 32-bit transfer independent of operand-size prefix; TR3--7 hardware commands | Pending instruction contract / S5; cache/TLB test effects system gate |
| I7 | 0FA2, candidate leaves 0/1 and explicit out-of-range policy, GPR outputs, flags, all privilege levels, old-profile rejection; ID-bit toggling through POPFD/IRETD | Pending / S6; exact 042B feature applicability and advertised VME system gate |
| I8 | Inherited flags paths: PUSHF/POPF, real/protected/v86 IRET, task load, 16-bit upper-half preservation, privilege masks, reserved bits | Pending affected-path regression / S6; AC exception/VIF/VIP behavior system gate |
| I9 | Inherited LOCK legality: primary ALU/XCHG versus FE/FF group extensions, existing bit operations, new I2/I3; old profiles and invalid register forms | Pending / S3 and whole sweep S7 |
| I10 | Default profile, old enum identities, old decoder inventory, reset provenance, FPU selection, retained family timing classifications | Pending regression / S2/S7; selected 486 reset/full timing system gate |
| I11 | 0FAA RSM and SMM entry/exit; CR0/CR3/CR4, descriptor accessed writes, WP and #AC | Explicit system gate, not omitted or implemented by this instruction task |

For I2/I3, the finite failure set is required instruction-byte fetch, segment
access, page access, read and write failure. Prove no partial architectural
register/flag publication; document the underlying bus's write boundary rather
than pretending CPU rollback can undo arbitrary device side effects.

I4--I6 must not gain successful execution with missing cache/TLB state. S5
must either connect a real existing operation demonstrated by inspection, or
retain the unimplemented execution path and transfer its full semantic forms
to the already named system receiver. Decode-contract work is not runtime
completion. This follows T558's existing instruction/system split; it does not
permit a new unsupported-instruction fallback, invented exception or dummy
cache. The Core machine admission gate stays closed in either case.

I7's candidate is the documented later SX contract, not a new claim about
physical PC110 stepping. No production machine may advertise its feature bits
before the system task proves their behavior and applicability. Unknown
optional features are source-gated, not silently removed from the objective.

T559 closes only after I1--I3/I7--I10 have their applicable direct proof and
retained regressions, I4--I6 have an honest per-form disposition and explicit
system receiver, all system gates remain enforced at production admission,
and required repository units/integrations and affected artifacts pass. It
does not close the subsequent system task or claim a runnable PC110.

## LOCK Finding And Planning Review

S2 reconciliation: I1 and the identity/decode portion of I10 are accepted
with [direct proof and artifacts](t559-s2-decode-bswap.md), implementation
`c93b778aa`. This does not discharge the remaining reset/system/timing or
whole-family sweep obligations in I10. I2 and inherited I9 transfer to S3;
all other rows retain their named planned receivers.

S3 reconciliation: I2 and ordinary ALU/XCHG LOCK classification in I9 are
accepted at c1a04cffb with [direct proof and artifacts](t559-s3-xadd-lock.md).
The table's pending entries above are the admission inventory; this dated
delivery reconciliation records their disposition. Remaining I9 forms still
require the whole sweep. I3 is admitted as S4, including write preflight on
both comparison outcomes; other rows retain their existing source/system
gates. The discovered inherited RMW read-first class is explicitly retained
in NXVM TODO, not counted as repaired by XADD.

S4 reconciliation: I3 is accepted at c4a08c66e with
[direct proof and artifacts](t559-s4-cmpxchg.md). Both compare outcomes retain
write permission/writeback, cached-address and alias semantics, and fault
rollback. I4--I6 now transfer together to active S5; the existing system
receiver still owns real cache/TLB effects. Remaining I7--I10 and external
boot qualification are not discharged by these arithmetic-family tests.

S5 executor reconciliation: I4--I6 have the explicit unavailable-state-owner
disposition permitted by this ledger, with their full encoding, privilege,
address and effect matrix accepted by the system proposal. The
[S5 review](t559-s5-system-instruction-boundaries.md) proves retained 386
transfers, illegal forms, existing privilege rejection and fetch rollback;
it does not qualify 486 cache/TLB effects. The old passive TR6/TR7 inheritance
remains a named limitation behind the existing production admission gate.
No successful no-op or invented exception replaces it. Complete units pass
596/596 per width. I7/I8 and the whole-delta/external-boot gates remain open.

S6 reconciliation: I7/I8 candidate CPUID/ID and affected flags paths are
accepted at e590a0dde with [source-qualified proof](t559-s6-cpuid-flags.md).
The existing Core gate still rejects production 486. Later-SX VME feature
encoding is not evidence of VME execution or exact PC110 silicon applicability;
both remain with the system receiver, alongside AC effects. I9/I10 whole
sweep and retained external boot qualification now belong to active S7.

S7 executor reconciliation: I9's complete retained LOCK sweep exposed and
repaired the bit decoder's bypass of the existing locked-register rejection.
The [S7 evidence](t559-s7-instruction-qualification.md) records all affected
forms, final 597/597 units per width and eight passing external groups with
rebuilt artifacts. I10's retained-family proof is reconciled; reset/full
timing and all other system gates stay with the complete system receiver.
This does not promote I4--I6 or I11 to implemented 486 semantics.

PRM section 13.1 permits the listed arithmetic/exchange memory-destination
operations without restricting their source GPR to AX/CX. Current
`PREFIX_LOCK()` applies FE's `/0..1` check to the preceding ordinary opcodes;
for example `F0 01 16 00 01` (16-bit ADD [0100h], DX) reaches that rejection.
The existing `cpu_legacy_lock_smoke.c` uses only AX in its 386 positive case,
so it cannot reject this implementation mistake. This is confirmed by source
control flow, not yet by a newly executed regression. S3 will separate ordinary
register-source forms from group-extension validation and cover both retained
386 and new 486 paths, without changing pre-386 transparent LOCK semantics.

The complete current equality/bounds sweep is classified above; the larger
numeric-comparison search is retained as an implementation checklist. No global
replacement is approved. Each implementation S rechecks its affected paths.

The initial search found 311 `PROFILE_80386` matching lines in five CPU files.
This is a search inventory, not 311 defects or 311 reviewed dispositions.
Comparisons such as `>= 80386` may correctly inherit behavior; equality checks
and 386-specific masks/timing need individual review.
