# T357 S2: CPU Timing Preview Mechanism Plan

## Purpose

The 80386 `Jcc` timing table assigns a taken branch `7+m` clocks.  Intel
defines `m` as the component count of the next executed instruction: each
prefix and ordinary byte counts once, while a complete displacement and a
complete immediate each count once.  This mechanism cannot be owned by
`machine.c`: the CPU executor is the only owner of instruction syntax and
architectural instruction fetch.

This plan defines the narrow prerequisite for T357's source-backed timing
ledger.  It is not an instruction-timing table and it does not add a prefetch,
wait-state, or speculative-execution model.

## Observed Routes

| Route | Current owner and behavior | S2 disposition |
| --- | --- | --- |
| Instruction fetch | `ExecInit()` in `src/core/machine/cpu_instructions.c` reads 15 bytes at CS:EIP and records the current instruction. | Retain as the executing fetch path.  Factor only the reusable lexical/format truth needed by a preview. |
| Decode/execute | `ExecIns()` applies prefixes and invokes the selected handler; individual handlers consume ModR/M, SIB, displacement, and immediate bytes. | Do not execute handlers during a preview.  The lexical owner must be shared with, or derived from, the executor's format authority rather than a machine-side parser. |
| Ordinary linear read | `_kma_read_linear()` reads page structures and payload through `_kma_read_physical()`. | Not usable for preview: it records CPU memory transactions and its normal completion can publish page A/D state. |
| Paging preparation | `_kma_prepare_physical_linear()` validates translation before `_kma_commit_physical_linear()` publishes A/D state. | Refactor into a nonpublishing fetch view that does not trace transactions, mutate CR2, retain an exception, or commit A/D. |
| CPU public helper | `core_machine_cpu_execution_read_linear()` temporarily saves `instruction_state.data.except`, but still calls the ordinary read route. | Do not reuse for preview; it is an architectural read helper, not a nonobservable lexer fetch. |
| Retirement timing | `core_machine_run()` in `machine.c` publishes elapsed time after a successful refresh. | Remains unchanged in S2.  The later ledger consumes the preview result before this sole publication point. |

## Required Owner Shape

The CPU owner supplies two private capabilities:

1. A nonpublishing instruction-fetch view that returns either a bounded
   sequence of instruction bytes or an unavailable result.  It never performs
   an architectural read, updates page A/D or CR2, starts a transaction,
   emits trace/diagnostic output, or delivers a fault.
2. A lexical instruction-format result that identifies prefix bytes, primary
   or `0F` opcode, ModR/M and optional SIB, and the displacement/immediate
   fields.  The component count follows the Intel `m` definition.  It does not
   validate semantic operands, execute a handler, or manufacture a fault.

The owner returns `unavailable` for a target that cannot be previewed without
architectural execution or whose format is not yet represented.  In the later
timing ledger this is a stable non-source-backed disposition, never a guessed
`m` value.  A source-backed taken-`Jcc` row is admitted only when preview is
available.

## Correctness Boundaries

| Boundary | Required result |
| --- | --- |
| Normal target | Exact component count; no CPU, RAM, port, page-table, trace, diagnostic, timeline, device, or scheduler mutation. |
| Segment/page/physical boundary | `unavailable`; the pre-existing architectural next instruction retains ownership of any later fault or A/D publication. |
| Invalid/unsupported encoding | `unavailable`; preview does not convert a later `#UD` into an early side effect. |
| Reset and split run | Preview state is transient; no retained cache or mutable secondary truth. |
| Prefix/ModR/M/SIB/displacement/immediate | Count according to the 80386 `m` definition: every prefix/ordinary byte is one component; the entire displacement is one component; the entire immediate is one component. |

## Similar-Issue Sweep Scope

The implementation reviews `ExecInit`, `ExecIns`, `_d_skip`, `_d_modrm`,
immediate and displacement readers, CPU instruction metadata, all linear and
physical CPU read paths, transaction begin/commit/cancel, paging preparation
and A/D publication, diagnostics, trace, debug, reset, and `Jcc` handlers.
Any non-CPU caller that needs a nonpublishing read is transferred rather than
borrowing this CPU-private mechanism.

## Transfers

- Actual prefetch queue behavior, branch-fetch overlap, and target-fetch bus
  cycles remain with the selected-profile cycle-exact task.
- Wait states, HOLD, DMA ownership, and physical bus visibility remain with
  bus-timed PC/AT operation.
- Instruction form clocks and any fallback disposition remain with the next
  T357 timing-ledger S.

## Implemented S2 Evidence

`core_machine_cpu_execution_preview_lexeme()` is private to the CPU execution
owner.  It clones only the CPU and instruction records, reuses `ExecInit()` for
the existing CS:EIP fetch and translation rules, and enables `preview_mode` on
that clone.  In preview mode the CPU memory path neither begins nor commits a
transaction, page translation does not commit A/D bits, and exception helpers
do not emit diagnostics.  A failed fetch or an unsupported lexical form
returns an unavailable lexeme; it does not alter the real instruction record.

The lexical result deliberately owns byte-format accounting only.  It does not
select an instruction handler, validate operands, apply semantics, or expose a
public API.  It uses the existing instruction metadata to reject unavailable
profile/opcode forms, and refuses `LOCK` rather than guessing the semantic
legality of a future target.  Thus it is not a second execution decoder and
cannot make a later fault or side effect occur early.

`core-machine-cpu-timing-preview-smoke` proves all of the following:

| Case | Observed result |
| --- | --- |
| Prefix/opcode/ModR/M/SIB/displacement/immediate | Correct Intel component accounting for `66 B8 id`, `67 8B` with SIB+disp32, near `0F 84 cd`, `0F 06`, `0F A4 /r ib`, and `F3 A4`. |
| Unsupported form | Truncated input, legacy `66`, invalid primary `82`, and `F0 90` are unavailable. |
| Accessible preview | A real 80386 reset-context fetch produces the expected lexeme without changing captured machine state, transaction counters, or trace count. |
| Inaccessible preview | A fetch window that crosses the 16 MiB physical-address boundary returns unavailable with the same no-publication checks. |
| Taken branch handoff | A real taken `JNZ -2` leaves EIP at its target; preview of that target reports the two `Jcc` components without changing the post-branch machine observation. |

The default PC/AT machine now maps the reset-vector alias at physical
`FFFF_FFF0` to the final 64 KiB of the first MiB.  This is the existing reset
contract implied by the CPU's reset `CS:EIP` and by the platform smokes; the
owner smoke proves it without adding a manual test mapping.  This full-gate
repair is platform-local and independent of timing semantics.

Two pre-existing descriptor-system smokes had called an arbitrary unmapped
IDTR base a "no IDT" fixture.  Their induced protected exceptions therefore
encountered a memory access failure rather than the intended no-IDT terminal
boundary.  They now use the existing empty `IDTR` (`base=0`, `limit=0`) form;
the change corrects fixture observability only and does not alter descriptor
or exception production.

The current developer artifact was built as
`build/output/nxvm_0_5_0357.exe`; SHA-256:
`CF8C225A4AD850C6EB9EA79A0C106171B0C0CF7340BE139EC8999F4DAFFA9B8C`.

The T345 ownership verifier was also reconciled with the current generated
matrix: 134 owner-test rows, 46 mixed/inherited rows, and 52 exact residual
production rows.  The newly surfaced `transaction.c` direct command is now an
explicit `machine-executor` residual rather than an implicit gap.

The final S2 current-gate run passed 235/235 tests.  The sweep classifies the
new lexical scanner as CPU-private format accounting only: it neither selects
handlers nor validates semantic operands.  `machine.c` has no timing consumer
yet; the later ledger S remains its sole prospective consumer.

## Source Record

- Intel 80386 Programmer's Reference Manual, section 17.2.2.3, defines the
  no-wait/no-HOLD/no-exception/aligned-memory timing assumptions and `m` as
  the next instruction's component count:
  <https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s17_02.htm>.
- Intel 80386 Programmer's Reference Manual, `Jcc`, gives short conditional
  branches as `7+m,3` and identifies the first value as taken:
  <https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/Jcc.htm>.
