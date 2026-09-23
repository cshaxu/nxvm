# NES Execution And Management: Shared Context

Retained historical M2 planning context. References to candidates and queue below
describe that planning stage; current M3 work is owned by Current and Queue.

The owner approved this proposal and requires SoftPC's App/Core/Lib/Common
composition with NXVM's core/machine organization and code style. The subsequent
instruction limits this delivery to design documentation and closure; source
implementation awaits a new active packet. The owner subsequently approved five
outcome-bearing implementation packages. This document is their shared context,
not a sixth candidate or a monolithic implementation task. Queue owns their order.
[Design/proof matrix](../etc/app-core-verification.md) owns the
finite coverage universe, and [App/Core](../etc/app-core-contract.md) plus
[CPU/bus/cartridge](../etc/cpu-bus-cartridge.md) supply the concrete contracts.

## Scope And Convergence

### Revised Component Boundary For Owner Confirmation

Owner confirmed closure of T4 after the flat source/test revision. The component
boundary below is the accepted design baseline; source work still requires its
implementation packet. Current records the actual admission state.

The owner's T4 reopening removes the separately named core/host layer. The
product has App, Core, Common and Lib. App owns entry/config/commands/composition;
Common owns session, executor and presentation orchestration; Lib owns native
capabilities. Core's NES behavior is grouped under core in NXVM style.

Within the flat Core component, driver.c implements Common's existing neutral driver. media.c
handles rom insert/eject through set_removable_media and Lib storage; debug.c
handles NES requests on the existing executor. Later input.c maps controls,
display.c produces pixels/text, audio.c converts/submits samples and pacing.c
applies NES speed policy using Lib primitives. These are machine responsibilities,
not a second host/executor/UI framework. Hardware files cannot depend on Common
or platform APIs. Lib/Common source and tests remain unchanged.

The concrete [component/file tree](../design/CODING.md#current-and-target-trees)
and [file responsibility table](../design/CODING.md#machine-file-responsibilities)
are the confirmed design surface. App starts with adjacent SoftPC-style files;
Core has no machine/host/debug subdirectories or forwarding facade. Explicit build file
groups preserve hardware/integration include boundaries within core.
M3/M4 file names describe future architecture only, not task or source admission.

Test placement follows the owner's component-level rule: test contains only app,
core, common, lib and integration. Core tests live directly in test/core;
there is no support root or machine subtree. Helpers remain with their owning
component tests; integrated scenarios belong in integration. This does not
modify the adopted shared test corpus.

### Implementation Coverage

Deliver the runnable M2 execution profile: App defaults/commands/composition,
Core driver, transactional strict mapper-0 ROM loading, all 151 documented CPU
opcodes and declared bus/interrupt behavior, copied debugging and ordered shutdown.
App exposes `rom insert <file_path>` and `rom eject` through Common's existing
removable-media interface, including replacement and failure preservation.
Apply the [NXVM-style source map](../design/CODING.md#nxvm-machine-style-reference):
machine-centered hardware files, opaque entry/private state headers, explicit
owners and bounded C routines. Review structural/style fidelity as part of BUILD;
retain Lib vocabulary and unchanged shared roots. No x86 implementation is imported.
The first source subtask must include a meaningful App/Core executable and the
required artifact/gates; do not create a dummy entry merely for an artifact rule.
Dynamic S admission groups cohesive vertical behavior after design review rather
than committing isolated libraries that cannot satisfy a runnable delivery gate.

Exit requires all M2 matrix rows ROM through PROMPT and BUILD to have direct
evidence on Windows x64/x86, independent shared suites unchanged, and a complete
coverage ledger for every admitted opcode/addressing/state/rejection class.
Original synthetic ROMs reach named checkpoints through the production path;
external games, PPU/APU/controller implementation, graphics/audio and broad game
compatibility are not claimed. No test or result may substitute "runs a loop" for
the finite matrix. Architecture, build identity and actual output must agree.

Stop on a required shared patch, absent contract, missing source rights or owner
scope expansion. Any such issue is resolved before dependent implementation.

## Incremental Delivery And Admission

The owner replaced the single-package plan with five deliverable packages:
vertical execution, complete CPU/bus, debugger workflow, cycle/interrupt
refinement, and reliability. Each builds on an already runnable product and
delivers working behavior, not an audit report. The first increment implements
one resumable cycle engine with a frozen small opcode set; later increments
complete and qualify that same engine. No disposable interpreter or duplicate
execution path is permitted. Unsupported commands/opcodes fail explicitly.

The [coverage transfer](../etc/app-core-verification.md#m2-package-proof-ownership)
maps every M2 obligation to its initial and final proof owner. Later full coverage
never excuses memory corruption, silent success, missing bounds or broken cleanup
in an earlier delivered capability. Every package runs its own focused tests and
all admitted regressions, and delivers usable x64/x86 task-version executables
under Execution. M2 final accuracy is not claimed by an earlier partial profile.

Admission allocates the next numeric T and one S packet dynamically, records the
whole package's coverage ledger and real acceptance cases, and creates its task
history. No S sequence or later milestone task list is preallocated here. Stop
on required shared changes, missing rights or unresolved implementation-critical
contract. This planning approval authorizes documentation; source admission is a
separate transition. The first packet can derive directly from the vertical
execution proposal without another standalone research/planning package.
