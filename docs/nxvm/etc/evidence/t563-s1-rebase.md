# T563 S1: Post-Refactor 486SX Completion Audit

## Baseline And Scope

Frozen source: `606ee84f4`, accepted T562. S1 changes documentation only.
It audits current owners and actual remaining connections, not every numerical
vector or fitted PC110 silicon. The original C01--C17 universe remains in
[T560 convergence](t560-s1-cpu-convergence.md); its original findings are
historical starting points, not assertions that accepted S2--S8 repairs vanished.
[T560 S9](t560-s9-selected-timing.md) retains the staged calculator source
grades and model assumptions. No new timing constant is approved by this audit.

## Current Owners

| Historical owner | Current source / corresponding test owner | Responsibility |
| --- | --- | --- |
| Core chips CPU | `src/x86-core/cpu`, `test/x86-core/cpu` | Architectural state, decoder, nominal/dynamic timing and actual access/dependency facts. |
| Core chips FPU | `src/x86-core/fpu`, `test/x86-core/fpu` | Existing neutral FPU contract; selected SX absence is CPU policy. |
| Core executor | `src/x86-core/machine`, `test/x86-core/machine` | Sole timeline, physical routes, pending completion, preview/budget and production CPU gate. |
| Core boards | `src/x86-nxvm/board-*`, `test/x86-nxvm/board-*` | PC reset mappings, clocks, peripherals and DMA wiring; no second CPU timing owner. |
| Core product/driver | `src/x86-nxvm/product`, `src/x86-nxvm/machine` and mirrored tests | Existing PC construction/publication; no PC110 admission or App CPU change here. |

T561's [path disposition](t561-s9-disposition.tsv) retains the broader move
inventory. Shared Lib/Emulator, MyNES and external firmware remain outside S1.

## Retained Functional Evidence

| Original rows | Preserved evidence and present receiver |
| --- | --- |
| C01, C10, C13, C14 | T560 S6 feature/exception evidence; current CPU CPUID/control, VME, debug and absent-FPU tests. Candidate silicon qualification is not an exact PC110 package claim. |
| C02--C04, C08 | T560 S2 base-system plus later state-owner evidence; current CPU reset/control/paging/descriptor owners. Core/Board production reset remains C17. |
| C05, C09 | T560 S3 alignment/RMW and S6 entry/nesting evidence; current CPU access/publication tests. Timing for those access paths still needs S3/S5 completion. |
| C06 | T560 S4 TLB evidence; current CPU TLB/control tests. Actual translation state is implemented; the full dynamic timing model is not thereby qualified. |
| C07 | T560 S5 cache evidence; current CPU cache/test-register tests. Cache behavior and cache penalty allocation are separate obligations. |
| C11, C12 | T560 S7 SMM/system evidence; current CPU system state and Machine executor tests. System elapsed-time/source assumptions remain S5. |
| C15 | T560 S8 opcode evidence and retained T559 instruction proof. Reuse that form universe for S2 allocation; do not replace it with a few successful programs. |
| C16, C17 | Explicitly incomplete staged timing and closed production gate, as verified below. |

The inherited evidence retains all original limitations. Fresh units below
establish the unchanged executable baseline, not exhaustive re-proof of each
manual claim. Final S8 must reconcile these rows again against actual changes.

## Confirmed Remaining Paths

All paths below are relative to the repository root. Function names identify
the audited owner without relying on line numbers that later edits will move.

| ID | Current evidence / remaining obligation | Receiving S |
| --- | --- | --- |
| R1 | `src/x86-core/cpu/cpu_timing.c` selects `x86_core_cpu_80486_source_instruction_cost` with compatibility origin. In `cpu_timing_model.c` that function still calls the explicitly unallocated compatibility recipe. Staged pure helpers are not selected runtime timing. | S6 |
| R2 | `x86_core_cpu_80486_nominal_instruction_cost` covers broad primary/escaped groups but returns false for other forms; variable IMUL/carry rotates have a separate L2 helper. WAIT, SALC/INT1/UMOV and RSM require explicit form/fault/system disposition against C15, not guessed fallback values. | S2, S5 |
| R3 | `x86_core_cpu_80486_operand_cache_penalty` is limited to ordinary integer/bit forms, rejects multiple data misses and unsupported access classes. Stack, strings, descriptor/control, failed and split paths need their own actual-access allocation at the same CPU owner. | S3 |
| R4 | `x86_core_cpu_80486_memory_penalty` requires table misses equal twice walks and update attempts equal the encoded walk updates. Its 13/21/28 baseline therefore does not cover cached/mixed walks or raced A/D attempts. | S3 |
| R5 | `cpu_instructions.c` records successful fetch bus ticks, cache/table misses, translation walks and GPR writes. Fetch overlap/refill/branch, waits and bus contention must use those real owners; the staged serialized fastest-burst assumption remains L2. | S4 |
| R6 | `x86_core_cpu_80486_decode_penalty` handles EA/prefix/AGI and REP continuation but declines redundant REP and REP on nonstrings. REP termination/fault/reset/debug/event lifetime and complete form applicability still need whole-path proof. | S4 |
| R7 | `cpu_instructions.c` accumulates interrupt/exception event subtotals; `machine.c` transports pending event completion. Nested failures, SMM/RSM and power elapsed time must join that one path, never fake a retired instruction. | S5, S6 |
| R8 | `x86_core_cpu_timing_maximum_ticks` lacks a selected 486 case. Existing Machine preview tests and `machine.c` budget/pending-completion paths do not qualify a complete selected-SX bound. Nonmutating preview and execution need one proven allocation and conservative budget treatment. | S6 |
| R9 | `src/x86-core/machine/machine.c` production CPU predicate still ends at 80386; profile gate and neutral-link tests reject SX. Existing executor tests deliberately rebind a created machine and are not production admission. | S7 |
| R10 | `src/x86-nxvm/board-base/machine_board.c` reset alias setup still selects 286/386; the reset alias switch groups SX with unsupported profiles. `machine_scheduler.c` already has SX cases, so mechanical broadening of every 386 check would be wrong. | S7 |

These are receiver/cohort gaps, not ten independent new frameworks. Keep CPU
facts at CPU, timeline/budget at Machine and PC reset wiring at Board. Retain
original table-style instruction code; no parallel interpreter or time axis.

## Source Decisions And Bounded Plan

S2 must enumerate the existing C15 forms and read their original source rows
before implementation; the absence of a nominal switch arm alone cannot
classify an encoding as legal or unsupported. S3 distinguishes architectural
transfer observations from Appendix E baseline cache/bus assumptions. S4 keeps
serialized fetch assumptions explicit. S5 must resolve system timing source
limits before qualification. Previously approved IMUL and variable RCL/RCR L2
models remain L2; this audit does not upgrade them by assertion.

The proposal now has S1--S8 with specific exits. S2--S5 finish cohesive owner
cohorts, S6 connects the sole selector/preview, S7 opens production only after
C16, S8 qualifies the original whole universe. Estimates are refined before
each implementation admission, rather than inventing a total line count now.

## Verification And Delivery

Fresh full repository unit builds/tests run on the existing x64/x86 Ninja
graphs with the persistent ccache directory, not new qualification directories.
Both widths passed 631/631 units: x64 73.87 s, x86 79.52 s. Local logs are
`build/t563-s1-{x64,x86}-{configure,build,units}.log`, with matching unit XML.
No new
integration, desktop/RDP or fitted-PC110 proof is claimed by this audit.

Production/test delta: zero. No public interface, manifest or artifact change
is needed for documentation-only S1; accepted PC 0562 and MyNES 0044 binaries
retain their existing executable-input provenance.

Actual-change review: documentation only; every original transfer cohort has
a receiving S, source limits remain explicit and C17 remains closed. The
documentation governance check and `git diff --check` pass. These checks do
not prove the still-unimplemented C16 numerical or runtime obligations.
