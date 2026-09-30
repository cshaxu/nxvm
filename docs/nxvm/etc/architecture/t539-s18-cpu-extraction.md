# T539 S18: Independent CPU Boundary

Baseline 25ec0f6c3. Current owns admission and acceptance. This document defines
the final nine-file CPU boundary. The owner's 2026-09-29
[S18-S54 decomposition](t539-cpu-work-packages.md) supersedes the original
single-batch delivery requirement. Intermediate packages may close against
their own complete briefs and full unit gates, but do not accept the CPU row.

## Ownership And Result

`x86/devices/cpu` owns architectural registers, decoder/instruction state,
segmentation/paging, prefetch, exception and retirement behavior, execution
budget and CPU-internal timing. Preserve the cohesive original handler tables
and formulas. Public callers receive an opaque instance, copied values and
bounded operations; neither board nor diagnostics receives its private layout.

NXVM keeps physical RAM/ROM routing, A20/reset aliases, PC port maps and latches,
PIC cascade, bus arbitration/transactions, external wait/page accounting and
the sole board clock/scheduler. The CPU must not hold their concrete pointers.
Do not move a machine-shaped context into Shared under another name.

Existing FPU state remains its independent opaque owner. CPU retains pairing
legality and operand operations; board composition binds the extension through
its declared public capability, without importing private peer headers.

## Execution Connections

Physical memory and I/O requests carry address, width, direction, access
provenance and bounded values/buffers. Observation intent and reset-fetch
identity preserve S17's single resolver. Board callbacks retain transaction
admission, cancellation and committed partial effects. Provider failure remains
distinct from architectural faults; do not promise rollback of consumed I/O.

Retain the baseline ordering of external-cycle BEGIN, transaction BEGIN,
transfer/CPU-local result publication, transaction COMMIT and external-cycle
COMMIT; error branches preserve their CANCEL order. A completed read callback
must not silently move a transaction commit ahead of a CPU operand update.
Use only the phases needed by this actual sequence, not a generic bus framework.

CPU samples a bounded interrupt input and requests an acknowledged vector.
NXVM connects the single/cascaded PIC and owns acknowledgement transactions.
No direct PIC scan or private port latch access remains in Shared CPU.

Diagnostics use copied instruction/register/access observations at the existing
retirement and fault points. There is one architectural state owner, not a
shadow CPU maintained by the board. Paused debugger operations use bounded CPU
operations through the existing machine adapter; no second execution path.

Retirement diagnostics distinguish the decoder's saved instruction-entry CPU
from the current CPU. Extend the existing copied architectural snapshot with
general registers, IP and FLAGS and select current versus instruction-entry
at its CPU owner. Retirement callbacks receive both copied values only when
an observer is installed; do not permit arbitrary running-state debug reads.
Paused debug capture retains its lifecycle admission. These are observations,
not additional mutable register owners or a newly retained execution history.

A synchronous board memory-write observer may use the existing CPU copied
CS/IP capture while running on the CPU's own callback stack. This is a borrowed,
non-mutating chip operation, not an unrestricted concurrent machine-debug API.
It must not execute the CPU, retain private state, or substitute a previous
retirement PC for the actual write-time position.

Entry-plan preparation keeps its original CPU-then-memory validation order.
CPU owns the short-lived prepared register candidate behind an opaque handle;
board validation either discards it or commits it before the existing preload
writes. It is not persistent mirror state. The stopped caller must finish the
candidate before executing, resetting or destroying CPU. Allocation failure
returns NO_MEMORY without publishing register or memory changes.

## Subtractive Work And Timing Split

Revalidated seven firmware providers all leave software_interrupt null. Delete
that slot, copied interrupt frame/result, CPU binding/helper and board forwarding
function. Keep live configure/reset/after-run and immutable ROM capabilities.
Real-mode INT uses the existing IVT path; protected-mode INT remains unchanged.

Separate CPU formulas/repeat state from board wait-window, external page and DMA
handoff accounting in cpu_timing_model.c. Runtime formulas require no generated
string catalog: the former key array served only a count assertion. Keep that
assertion with the actual NXVM metadata-runner consumers, not in Shared CPU.
Independent chip tests must not acquire NXVM source/build/assets as a hidden
prerequisite; board manifest runners retain their existing provenance inputs.

## Migration And Proof

Implement the boundary and consumer changes before the final structural move;
temporary worktree incompleteness is not a delivered compatibility layer.
Map every original CPU test to chip-local or board responsibility. Keep mixed
board scenarios in NXVM while testing chip behavior independently. Preserve all
CPU families, timing classifications, fault paths, FPU cases and debugger data.

Review mechanical moves separately from executable differences. Add static
checks against App/private-peer dependencies and the retired interception chain.
Current specifies full receiving verification, target-separated commits and
artifact identity. The final evidence must enumerate all original CPU files,
callers and test dispositions, not infer completeness from a successful boot.
