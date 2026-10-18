# T2 S33 Ordinary Native Control Nominal Clocks

Baseline `3abcbd00c`, after accepted S32. This consumes the ordinary native
control source subfamily of original V15. Final bounded qualification passes;
coordinator acceptance remains. Complete V30 physical time, Machine/board and
playable game remain open.

## Primary Source And Complete Family

NEC October 1986 V20/V30 User's Manual, SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`.
[Original source](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf).
Printed 12-142..146,154..166, PDF 187..191,199..211 were rendered and visually
inspected. The original section 12-1 prefetched, zero-wait nominal assumptions
remain; no manual payload, external code or ROM is imported.

| Complete ordinary native family | Nominal source clocks |
| --- | --- |
| Sixteen conditional branches | 14 taken / 4 untaken |
| LOOPZ / LOOPNZ | 14 taken / 5 untaken |
| LOOP / JCXZ | 13 taken / 5 untaken |
| Relative short/word JMP | 12 |
| Direct far JMP | 15 |
| Indirect near JMP register / memory | 11 / 20 even,24 odd |
| Indirect far JMP memory | 27 even,35 odd |
| Relative near CALL | 16 even stack,20 odd |
| Indirect near CALL register | 14 even stack,18 odd |
| Indirect near CALL memory | 23 all-even,31 all-odd |
| Direct far CALL | 21 even stack,29 odd |
| Indirect far CALL memory | 31 all-even,47 all-odd |
| Near RET / RET cleanup | 15/20 even stack,19/24 odd |
| Far RET / RET cleanup | 21/24 even stack,29/32 odd |

The source's memory-CALL rows distinguish all-even/all-odd addresses.
Mixed pointer/stack parity has no documented row and remains zero/unattributed,
not an inferred per-transfer slope. Illegal far-register CALL/JMP encodings
remain prohibited by the existing shared form owner. No undocumented alias
is admitted by a timing selector. Prefix envelopes, software/asynchronous
entry/return, mode/8080 and full bus/prefetch/wait accounting retain original
V15 receivers. Source-known ordinary control does not retire those classes.

## Single Decision And Publication Owner

Only the existing V30 nominal selector in `cpu_timing.c` changes. It reuses
`x86_core_cpu_timing_branch_taken()`, the already-owned pure timing decision
over original flags and count; no predicate or executor is copied. In
particular, taken is not inferred from PC inequality: a taken displacement
zero branch reaches the same next PC as an untaken branch but costs differently.
Original SP selects stack alignment even after POP/RET changes SP, and original
decoded pointer offset selects memory alignment. Cleanup values never replace
the original stack context.

The NEC nominal rows have no 386 next-instruction lexical-component term.
Selection therefore adds no target preview, observational read, target fetch,
or Intel EA/byte-callback cost. Missing/invalid next code cannot turn the
current committed instruction's nominal subtotal into a guessed 386 formula.
Real subsequent execution/fetch still belongs to its existing executor.

Existing CONTROL/BRANCH/TAKEN, MEMORY, MODRM/effective-address and ODD_WORD
inputs describe the calculation. Existing opaque opcode/group keys distinguish
FF /2..5. Two local calculation booleans are transient facts, not persistent
state or a second control machine. No production helper, public API/enum/layout,
CPU semantics, bus transaction, queue, thread or Lib/Emulator change is added.
The existing selector publishes once and its common refresh/failure lifecycle
remains unchanged. Existing Intel and earlier nominal selectors retain their
real profile/form responsibilities. Total V30 time remains unallocated,
maximum zero and Machine admission closed.

## Owned Proof And Initial Corrections

The existing Control Flow unit retains all 463,982 successful inputs:
sixteen conditions across flags and every short displacement, all word
relative displacements, all four loop conditions and count wrap, direct/
indirect near/far forms, every legal ModRM, original SP/register aliases,
segments, PC/stack/pointer wrap, overlap and cleanup. Original complete CPU,
physical transfer/phase, accepted RAM prefixes and every provider refusal
remain. A semantic-input oracle adds independent clocks, origins, opaque keys,
formula facts and explicit unknown cases; it does not duplicate production
opcode selection. Original branch-result preparation now retains its taken
fact for that oracle instead of computing a second condition.

Actual CALL -> RET paths also check return clocks. Every scoped read/write/
fetch refusal adds no-completed-timing checks while keeping original rollback
assertions; preview refusal remains observational. Successful execution
asserts zero added observations after the original preview counter is reset,
proving no timing-induced target read. Target availability is not used as a
test precondition; zero added observations proves no target-read dependency.
This is not a new downstream target-fetch qualification.

Two initial candidate test assumptions were wrong:

Observed markers were `V30:CONTROL-FLOW:FAIL form=0 condition=0` with
identical actual/expected PC and `V30:NOMINAL-ALU:EXCLUDED index=10 stop=1 ticks=0`.

- The first new observation assertion compared with the earlier preview count
  even though the existing test resets it before execution. It is corrected
  to require zero new execution observations, not to ignore the check.
- The prior ALU exclusion used ordinary register CALL, now intentionally
  source-allocated. Its replacement far-register CALL is not an executable
  timing-unknown alias: the unchanged `_kdf_modrm_form_valid()` rejects FF /3
  and /5 with mod=3 before effects. Existing form-admission tests independently
  enumerate those encodings. The exclusion now explicitly expects that
  existing refusal/no completion, as it already does for F6/F7 /1. No
  production admission/execution change was made to satisfy the test.

The ALU unit retains all 917,460 semantic inputs and fifteen neighboring
boundaries; its sole changed exclusion is not an ALU semantic/oracle rewrite.
All prior transfer/stack/port/flag/entry/mode/Intel proof is retained.

## Similar-Issue Sweep

Queries:
- `rg -n 'timing_branch_taken|control_stack_short_branch_taken|next_lexeme_components' src/x86-core/cpu`
- `rg -n 'CONTROL|control|stack_penalty|formula_inputs' src/x86-core/cpu/cpu_timing.c`
- `rg -n 'taken|fail_|observations|timing' test/x86-core/cpu/cpu_v30_control_flow_smoke.c`
- `rg -n 'V30|timing_maximum_ticks' src/x86-core/machine src/x86-core/cpu`

Every ordinary native control source form has the same preparation/selection/
publication owner. Existing pure decisions serve both profiles; Intel
source/next-term calculations remain genuinely different and are not applied
to NEC. Software/async/8080/prefix and source-unknown mixed-alignment classes
remain original explicit V15 work. Common form admission owns prohibited
encodings, not a new timing-driven admission path. Original executor/rollback/
preview/max/Machine consumers remain unchanged.

## Counted Change And Verification Boundary

Tracked C/H/CMake diff against baseline, excluding manifests/docs/artifacts:
production +34/-4/net +30; tests +73/-6/net +67; code +107/-10/net +97.
Manifests separately +3/-3/net zero. No registration or framework is added.
Positive test code is a source oracle and checks in the existing real path,
not another CPU model. Production is below the estimate because the pure
decision and source stack alignment owner are reused. Final qualification
and committed-diff review must reconcile these numbers before acceptance.

## Final Qualification

Shared source/test delivery is `1857f80d6`; NXVM receiving publication is
`d544c5714`. These target-separated complete deliveries are pushed.

Both original cached Ninja all-target builds pass. Complete repository units
pass 678/678 per width, including 191 X86 Core tests, zero failures/skips,
in 297.33 seconds x64 and 132.36 seconds x86. Both eighteen-test related
groups and all specialized gates pass, including 498 retained strict rows and
global Types coverage of 1,388 code/script inputs. Existing warning exceptions
remain unchanged. No assertion, budget or production admission is weakened.
Initial candidate test errors are preserved above; no failed final full suite
is hidden by a retry.

Eight current PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations plus restored-default identity pass per width. Both original
graphs are default and profile switches recompile zero CPU objects. All ten
deployed PE/hash identities pass; MyNES 0044, owner INIs and protected inputs
are unchanged. All owned qualification handles are terminal. Measured
[results](t2-s33-results.json) and [artifacts](t2-s33-artifacts.json) retain proof.
Fresh complete external/native integration, manual desktop/RDP/audio, physical
CPU timing and playable game are not claimed.

## Complete Admission Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation, MyRaidenII M1 T2 S33, next unused S after accepted S32. |
| Admission And Approval | Owner authorizes automatic continued governed single-person coordinator/executor MyRaidenII work. Coordinator admits ordinary native control-flow nominal clocks on 2026-10-18, targets Shared CPU/tests, NXVM eight receiving releases/status and MyRaidenII evidence. Existing V30 CPU authorization applies; no Lib/Emulator/MyNES source or new public contract. |
| Objective | Consume the complete ordinary native control source family of V15: sixteen Jcc, LOOP/LOOPZ/LOOPNZ/JCXZ, direct/indirect near/far JMP/CALL and ordinary near/far RET with/without cleanup. Use the sole existing source selector and already-owned pure branch decision; no duplicate conditions or execution logic. |
| Non-goals | No software/asynchronous interrupt timing, 8080/mode entry, dynamic arithmetic/string/prefix timing, new model/API/enum/layout/state/helper framework, CPU semantic change, Lib/Emulator/MyNES edits or Machine/game admission. Nominal source subtotal remains unallocated total time, maximum zero. |
| Reference Baseline | Clean pushed 3abcbd00c after accepted S32. Original NEC October 1986 manual SHA FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C, printed 12-142..146 and 12-154..166, visually inspected PDF 187..191 and 199..211. S20 complete control semantic/failure matrix and S30..S32 source ownership. |
| Candidate Proposal | proposals/m1-v30.md and original V15 ordinary native control source receiver. Conditional clocks reuse x86_core_cpu_timing_branch_taken(), not PC equality or copied conditions; all original register/count/stack facts select nominal rows. No next-instruction preview or Intel EA addition: NEC rows already include their nominal work. Mixed pointer/stack alignment stays zero/unattributed; far-register encodings retain their existing rejection. |
| Files And ABI Surface | src/x86-core/cpu/cpu_timing.c and manifest; test/x86-core/cpu/cpu_v30_control_flow_smoke.c and directly affected ALU timing-only exclusion plus test manifest. Estimate production +55..85/-5..15, tests +70..110/-5..15; no test registration/public declaration. NXVM artifacts/status and MyRaidenII records in separate target Ps. |
| Applicable Rules | Product guide/design/roadmap/source policy and original ledger, Execution S/P/roles/coverage/artifacts, Architecture/Coding skills and rules, Documentation, read-only PDF source review; receiving NXVM guide/design/coding/source policy. Lib Types, single decision/selection/publication owner and actual-diff review. |
| Verification | Reuse all 463,982 ordinary native control inputs with independent source costs, taken/untaken even at zero displacement, counts/flags, every ModRM, width/segment/wrap/alias/overlap/cleanup context. Retain CPU/RAM/physical plan proof and every read/write/fetch/preview refusal; extend actual CALL-to-RET proof to clocks. Unknown mixed/prefix classes remain unknown and prohibited far-register forms remain rejected; selection adds no observational target reads. Preserve full prior V30/Intel cases. Both original cached Ninja all-target builds -j8, complete repository units -j4, specialized gates/498 strict rows, fourteen PC receiving integrations and restored identity per width, ten deployed PE/hashes and unchanged owner inputs. |
| Expected Markers | Existing V30:CONTROL-FLOW:OK cases=463982 retained with V30:NOMINAL-CONTROL:OK cases=463982; prior transfer/ALU/flags/stack/port and arbitration/completion markers unchanged. Both complete dual-width suites zero failures/skips; no source-cost result opens V30 Machine or maximum. |
| Asset Needs | Original synthetic ROM-free programs and already pinned manual only. Eight current PC 0564 EXEs rebuilt/deployed; MyNES 0044 unchanged, ten identities verified. No protected payload, media/INI/snapshot change or MyRaidenII EXE. |
| Reporting Requirements | Initial source/ownership confirmation and estimates; progress/real failures while executing; final actual production/test added/removed/net, retained paths, source/disposition and qualification evidence, artifacts/commits and all remaining V30/game boundaries. |
| Stop Conditions | New public/source-model requirement, changed old Intel semantics, owner input mutation or failed qualification blocks acceptance. Do not fabricate missing rows, weaken assertions/budgets or turn partial nominal clocks into elapsed-time eligibility. |
| Exit Criteria | Entire admitted control source family has source-allocated or original source-unknown disposition and actual semantic/failure/key/clock proof at one owner. Both widths complete builds/units/gates/receiving/artifact/doc checks pass; target-separated complete Ps pushed and actual committed-diff coordinator acceptance. Close only S33; original complete T2/M1 stays open. |
| Original Owner Request | Implement complete playable MyRaidenII under governed single-person/two-role automatic continuation, including full NEC V30 and required facilities; CPU tests or partial clocks never replace game outcome. |
| Similar-Issue Sweep | Audit all control opcodes/subops, taken predicates/count wrap, source pointer versus old SP, direct/indirect/near/far/cleanup variants and timing/preview/max/Machine consumers. Reuse existing pure decision, preserve original executor/bus rollback, keep software/async/8080/prefix/full physical time at their original V15 receivers. |
