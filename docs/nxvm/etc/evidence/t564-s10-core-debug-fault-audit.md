# T564 S10: X86 Core descriptor, Debug CR and CPU-fault audit

## Disposition

No current production defect was reproduced.  The three reviewed contracts
already have one owner and one outward boundary; adding a new snapshot,
exception route or Debug compatibility policy would duplicate an existing
mechanism.

### Descriptors

`x86-core/cpu` alone owns the legacy descriptor representation and its
`_GetDesc_*` / `_IsDesc*` helpers.  They are internal implementation
vocabulary, not a public component contract.  `x86_core_cpu_debug_copy_segment`
performs the sole conversion to the semantic copied Core Debug snapshot;
`x86-nxvm/machine/debug_adapter.c` performs the sole protocol adaptation for
X86 Debug.  The debugger formats that copied data and never observes a CPU
layout.  Renaming the historical internal vocabulary would touch many callers
without changing a boundary, behavior or ownership, so it is not a justified
repair.

### Paused Debug control-register edits

Core Machine admits register edits only at its stopped, paused or faulted
debug boundary and rejects an in-progress CPU retirement.  CPU validation
builds a candidate state before committing the live cache/bus effects.  The
shared CPU owner invalidates prefetch/TLB state after CR0 or CR3 edits; an
invalid candidate does not commit descriptor/cache state.  Existing Core tests
cover cache, paging, descriptor and Debug paths.

This is an administrative paused-Debug operation, not a guest `MOV CR`
instruction.  It deliberately does not import guest privilege, #UD or all
reserved-bit behavior into the Debug command.  On the selected 486SX profile,
CR4 validation is limited to the implemented VME bit.  Outside that selected
hardware contract, retained raw CR storage is not a claim of hardware support
or silicon-accurate guest behavior.  No user-visible change is justified
without an approved cross-family Debug policy.

`XCREG` intentionally presents the historical CR0/CR2/CR3 display.  Individual
supported control registers remain available through the established register
protocol; adding CR1/CR4 to the copied aggregate solely for this audit would
widen a public surface without a consumer.

### CPU fault reporting

CPU publishes one copied diagnostic snapshot at the execution boundary.
Deliverable exceptions are recorded separately from terminal faults.  Core
Machine completes pending timing/retirement work before publishing a terminal
fault result and transitioning lifecycle to `FAULTED`.  `x86-nxvm/machine`
has the sole receiver that copies this run result for product diagnostics; it
does not make a second fault channel.  Existing paging and FPU tests distinguish
delivered exceptions from terminal faults, including the page-fault CR2 fact.
No lost detail or contradictory lifecycle result was reproduced.

## Focused verification

The following focused CTest selection passed on both persistent Ninja Release
trees: x64 14/14 in 2.07 seconds; x86 14/14 in 2.29 seconds.

- Core CPU cache, paging and fault-event tests;
- Core Machine retirement, memory inspection, descriptor, paging and Debug
  tests;
- Core Machine CPU-fault diagnostic test;
- X86 Debug mapping/product tests; and
- X86 NXVM Debug budget, board fault-diagnostic and task-switch/paging tests.

This is an audit result, not a complete hardware qualification or a claim that
all possible raw control-register states are guest-legal.  No source, test,
interface, manifest, binary, INI or media file changed.
