# T390 S2: Model-40 BYOB Observation Contract

`M5:T390:S2:MODEL40-BYOB-OBSERVATION-CONTRACT:OK`

## Decision

The selected Model-40 corpus must be observed through the shared Core retirement
boundary before any 80386 form may be called physical-eligible for that
corpus. The current trace provider is insufficient for this purpose:
`CURRENT-TRACE-INSUFFICIENT`. It records a post-publication `CPU_RETIRE` event
with a linear PC and truncated tick value, but does not identify the retired
instruction bytes, prefix/decode and execution context, timing-row/classifier
disposition, or a rejected source-unallocated success. The existing physical
rejection occurs before retirement publication, so it cannot appear as a
`CPU_RETIRE` event.

This is a design and observation boundary only. It does not execute an external
ROM or medium, select the physical contract, or assign a timing row.

## Corpus Boundary

The future run consumes an owner-supplied, valid Model-40 BYOB ROM pair and an
owner-supplied bootable 1.2 MB floppy medium outside this repository. It uses
the selected Model-40 backbone: 80386 CPU profile, 1 MiB memory, no FPU,
selected 1.2 MB floppy profile, no optional fixed disk, cold reset, and the
normal BYOB validation/mapping route. Local file paths, hashes, bytes, vendor
catalogue entries, provenance text and raw trace stay external; the repository
retains only the normalized conclusion and project-owned focused regressions.

The corpus is not an emulator comparison and not a universal 80386 workload.
It is the finite firmware-and-boot path from cold reset to the owner-designated
first boot-sector control-transfer checkpoint. The owner records that external
checkpoint identity with the run record; a run that faults, requests stop,
exhausts a budget, or reaches an unqualified success ends at that terminal
reason and does not satisfy the checkpoint.

## Required Normalized Observation

For every successfully retired instruction through the checkpoint, the later
shared capture must make one ordered record available before physical time can
be published. Each record requires:

- pre-execution linear PC plus enough raw instruction bytes to identify opcode,
  escape, prefix sequence and ModR/M/SIB/immediate extent when present;
- CPU profile, execution mode, effective operand/address size, CPL and the
  captured state used by the selected timing classifier (including repeat,
  privilege/descriptor or dynamic-operand facts where applicable);
- exact timing-row identifier and tick result, or the explicit
  `source_timing_unallocated` disposition; and
- retirement outcome and terminal reason, with the elapsed/timeline values at
  the publication boundary.

The normalized record is an observation of NXVM's own classifier, not an
external timing oracle. Later T390 work maps a frozen observed form/context to
an Intel-primary row and adds project-owned regression coverage; an observed
unallocated success remains nonphysical and stops under the physical contract.

## Run Containment

A later launch must use a fresh ignored directory below `build/`, reserve twice
its hard 8 MiB raw-output budget before launch, and own the complete launched
process tree. It has a 30-second wall-clock budget, a 5-second no-progress
budget, and a 250,000-retirement budget. The recorder or harness stops before
8 MiB; on first excess the harness terminates the complete process tree. It
records the terminal checkpoint/reason and final output size outside the
repository, waits for process exit and closed handles, then deletes the raw
trace. It neither reuses a trace path nor begins a second run while an owned
process or trace remains.

These are containment limits, not expected boot performance. The only retained
result is a compact evidence conclusion and, when required, a project-owned
minimal reproducer that contains no vendor firmware or guest-media content.

## Current Owner Sweep

`core_machine_instruction_cost()` resets and resolves
`source_timing_unallocated` through every current string/I-O, dynamic,
secondary, privileged, primary/control-stack and CPU-profile fallback route.
The physical contract faults an unallocated success before
`core_machine_publish_elapsed_ticks()`. That publisher emits the sole current
`CPU_RETIRE` event after incrementing elapsed ticks. The public trace event has
only address/value/detail fields; the current event uses address for a linear
PC, value for a 32-bit tick value and zero detail. It cannot carry the required
classification result or source context.

`CORE_MACHINE_TRACE_CAPACITY` is 32, but the implementation flushes to its
provider synchronously after each record. It is therefore neither a bounded
corpus recorder nor proof of loss-free external persistence; a callback can
still create unbounded host output. Capacity is not the reason for this
contract's rejection of the event format. The missing pre-publication fields
and rejected-success record are decisive.

The Model-40 BYOB composition validates and loads external ROM through its
existing VM/profile boundary, then freezes a neutral deterministic clock plan.
No Model-40 profile route selects physical retirement today.

## Next Receiver

T390's next bounded receiver is a shared Core production observation/capture
slice. It must expose a lifecycle-owned, opt-in diagnostic observer rather
than a test-only instruction API; preserve the existing generic trace provider
and its ABI; produce the required pre-publication exact-or-unallocated record;
and prove reset/failure atomicity, disabled-path neutrality and physical
nonpublication. Only after that capture exists may the contained external run
be launched and its finite form/context matrix evaluated.