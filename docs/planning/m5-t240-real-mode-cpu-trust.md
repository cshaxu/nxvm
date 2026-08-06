# M5 T240: Real-Mode 8086 Trust Corpus

## S1: Contract And Evidence Map

**Status:** S1 complete; later implementation awaits owner-approved corpus
slice.

T240 verifies the existing 8086 executor before it admits any repair.  Its
scope is the 8086 real-mode instruction behavior actually needed by the
default PC/AT ROM, focused boot fixtures, DOS boot path, and retained device
regressions.  It does not claim full 80186, 286, 386, protected-mode, paging,
FPU, or cycle-accurate coverage.

### Authority And Boundaries

The primary architecture authority is Intel, *The 8086 Family User's Manual*,
October 1979, order `9800722-03` (archived at
<https://bitsavers.org/components/intel/8086/9800722-03_The_8086_Family_Users_Manual_Oct79.pdf>).
The local Bochs 2.6 compatibility checkout may be inspected only to frame a
bounded diagnostic question after an owned probe exists.  No Bochs source,
test, trace, build input, or runtime dependency is imported or used as
acceptance evidence.

| Concern | Owner / rule |
| --- | --- |
| CPU state, decoder, exception outcome, execution | `core/machine` CPU executor through `core_machine_run()` |
| System topology, ROM, device providers | VM composition/default profile; it does not bypass CPU semantics |
| Product acceptance | Default `80386 + no FPU` FDD/HDD/DOS/Console/debugger matrix remains separate from strict-8086 claims |
| Time | Existing T217/T218 core elapsed-tick contract; T240 does not change it |

An instruction is classified as one of: **8086-required pass**; **8086-required
fault**; **profile-dependent extension**; or **deferred**.  A profile-dependent
extension neither widens the 8086 claim nor becomes a defect merely because a
strict 8086 rejects it.

### Current Evidence Map

| Class | Current real path / focused evidence | S1 classification |
| --- | --- | --- |
| Reset, register/segment setup, ordinary data movement and arithmetic | Default ROM and boot fixtures; `core-machine-real-mode-tick-smoke` MOV case | 8086-required pass; expand semantic probes before asserting full family coverage |
| Segment prefixes and near control transfer | ROM/firmware streams; T218 segment-prefix case | 8086-required pass; prefix must remain part of one instruction |
| `INT`, IVT dispatch, `IRET`, FLAGS/stack return | ROM services, IRQ delivery; `cpu-int-ivt-smoke` | 8086-required pass; add an owned round-trip probe that observes return state |
| Immediate and DX port I/O | ROM/device paths; T218 OUT case | 8086-required pass; add owned read/write/provider-sequence probe |
| String data movement and direction/`REP` behavior | ROM and DOS software paths; current executor has `MOVS`/`STOS`/`CMPS`/`SCAS` handlers | 8086-required pass; add bounded DF/REP semantic probes |
| `HLT`, wakeup, and interrupt boundary | T218 HLT case and retained IRQ regressions | 8086-required pass; retain current focused evidence, add no new timing model |
| Undefined encodings and diagnostic retention | T218 `66h` strict-8086 case; CPU fault diagnostic and T214 runner handoff probes | 8086-required fault; preserve `STOP_FAULT` and first diagnostic until reset |
| `C1 EA 04` (`SHR DX,4`) in the supplied DOS image | T209 focused CPU-profile gate and FDD trace | Profile-dependent 80186+ extension. Strict 8086 `#UD` is expected; default 80386 FDD prompt is the product acceptance path. |
| 80186-only string I/O and other later forms | Decoder metadata/profile gates | Out of the 8086 claim; classify only when a retained real path requires it |
| `ESC`/`WAIT`, 8087 behavior, `#NM` | Current no-FPU profile handling | Deferred to a dedicated FPU task; no T240 inference from an FPU skeleton |
| Protected mode, paging, descriptor/task machinery | None | Explicitly deferred |

### S1 Probe Plan

The T218 smoke is retained as a narrow time-attribution check; it is not a
complete CPU proof.  Before any CPU repair, the next admitted subtask must add
one project-owned, reset-vector-based `8086` corpus smoke with a bounded
instruction budget.  It must use only the existing `core_machine` public
configuration/observation boundary and must cover:

1. Segment register setup plus a segment-overridden memory operation.
2. `REP MOVSB` and `STD`/`CLD` direction behavior with source, destination,
   count, and copied bytes observed.
3. An `INT` -> IVT handler -> `IRET` round trip, including stack/FLAGS return.
4. Immediate and DX `IN`/`OUT` through a test-only port provider, including its
   transaction sequence.
5. A strict-8086 undefined/later-prefix negative case that preserves the T214
   fault outcome.

The retained T209 profile-gate remains the separate `C1 /5 ib` 8086-reject /
80186-accept test.  System evidence must continue to run the default-80386
FDD/HDD boot, DOS prompt, `MEM` sample, CGA/EGA, keyboard, Console, and
debugger matrix.  No trace collector, alternate executor, host-clock path, or
new runtime interface is admitted.

### Similar-Issue Sweep

S1 is a contract/inventory subtask and changes no defect or runtime source.
The required sweep for a later repair covers all `src/core/machine` decoder and
execution paths, CPU-profile metadata, focused CPU tests, system fixtures, and
task/governance records that make the same semantic claim.  Each hit must be
fixed, shown inapplicable, or deferred with an admission condition.

### S1 Exit And Stop Conditions

S1 exits only after this map is reviewed and the next subtask has a narrow
probe contract.  Stop and request a scope change for any finding that requires
protected mode, FPU emulation, a second executor, a VM/profile CPU shortcut,
unbounded instruction tracing, or a Console/debugger/startup behavior change.
S1 produces no runnable artifact because it changes neither source behavior
nor an executable product path.

### S1 Findings

Source inspection confirms that the existing core executor has owner-local
handlers for the string, interrupt-return, port-I/O, and halt families named
above.  That is an implementation inventory, not semantic proof.  The retained
T218 smoke proves only MOV, one output form, `INT`, a prefix, `HLT`, and a
strict-8086 later-prefix fault; it cannot establish the `REP`/direction,
`IRET`, or port-provider edge cases required for a broader 8086 claim.

No reproducible 8086 defect was found in S1, so no CPU behavior was changed.
The next subtask should admit exactly one bounded owned probe slice before
considering any repair:

| Candidate slice | Scope | Exit evidence |
| --- | --- | --- |
| S2a: strings and direction | `REP MOVSB`, `STD`/`CLD`, register/count and copied-byte observations | One reset-vector core probe plus current matrix |
| S2b: software interrupt round trip | `INT`, IVT vector, stack image, `IRET`, restored FLAGS/CS:IP | One reset-vector core probe plus retained fault handoff evidence |
| S2c: port transaction semantics | Immediate and DX `IN`/`OUT` through a test-only provider | Exact provider transaction sequence and current matrix |
| S2d: corpus closure | Run focused probes against default product regressions and classify any defect | Full current matrix and compact T240 evidence update |

Each slice stays in T240, uses the same `S` sequence selected by the owner,
and performs the mandatory similar-issue sweep if it repairs a defect.
