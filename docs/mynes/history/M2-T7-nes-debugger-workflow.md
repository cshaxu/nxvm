# M2 T7: NES Debugger Workflow

## Admission And S Plan

T7 starts after accepted CPU/bus execution. It delivers the approved cooked
Console workflow through the existing Common lifecycle and the one production
machine. Work proceeds in these outcome-bearing subtasks:

| Subtask | Outcome | Closure proof |
| --- | --- | --- |
| S1 | Establish the public Core debug request/response boundary and paused-lease rules. | Every supported request accepts only valid copied data, rejects invalid state or layout, and leaves no mutation on rejected input. |
| S2 | Connect Core operations to Console commands and user-visible records. | Fixture-driven `regs`, `mem`, `poke`, `disasm`, `step`, breakpoint and reset commands use the real dispatcher and produce stable output. |
| S3 | Complete breakpoint, stepping, trap and resume semantics. | The real machine stops, bypasses/rearms breakpoints, preserves BRK/trap facts and resumes using its only CPU path. |
| S4 | Run the closure matrix and transfer remaining timing/reliability obligations. | All prior and debugger tests pass on x64/x86; unsupported or deferred dimensions name their receiving package. |

## Scope And Boundaries

The accepted proposal owns OBSERVE, PEEK, POKE, STEP, BREAK_SET, BREAK_LIST,
WARM_RESET and the specified unsupported OUTPUT_SET/BIND_SET responses. App
parses the monitor vocabulary and displays copied records. Core validates the
request and performs machine work. Common grants and validates paused leases;
Lib and Common source remains unchanged.

No second interpreter, assembler, private CPU setter, timing model, graphics,
audio or controller implementation belongs here. Poke is RAM-only, peek is
side-effect-free, and every observation is bounded and copied before the
Console sees it.

## S1 Result

S1 added the sole Core debug dispatcher behind Common's existing paused lease.
It implements copied OBSERVE, PEEK, RAM-only POKE, STEP, BREAK_SET, BREAK_LIST
and WARM_RESET records, and validates the two gameplay-profile operations as
explicitly unsupported. Core owns a sorted sixteen-entry execution breakpoint
set; normal run pauses at a hit while step keeps using the existing CPU path.
The integration fixture proves protocol layouts, RAM persistence through warm
reset, step results, breakpoint listing, invalid RAM writes and unsupported
profile requests through the real Common rendezvous.

## S2 Result

S2 connects the protocol to cooked-Console commands. `regs`, `mem`, `poke`,
`step`, `break`, `delete`, `breaks` and `reset soft` all acquire a fresh paused
lease through the real App/Common/Core path. `disasm` reads a bounded copied
range and renders the 151 accepted opcode forms from the project ledger; every
other byte remains `.byte`. App-only memory and disassembly count defaults are
validated and bounded. The integration fixture asserts parser output, state
rejection and Core effects without a private hardware handle.

## S3 Result

S3 proves execution breakpoints through Common's worker and the production CPU:
a hit at the current PC stops before fetch, the next resume bypasses that address
once, and a loop returning to it stops again. No instruction patch or alternate
interpreter is involved. App observes a latched guest trap before allowing run
or step and requires reset or media replacement for recovery.

## Initial Verification Plan

The task begins with request-contract fixtures, then command-to-dispatch
integration fixtures, then stopped-machine scenarios. Each stage retains the
full product regression suite. The closure audit additionally builds and tests
the current product target for both supported Windows architectures.
