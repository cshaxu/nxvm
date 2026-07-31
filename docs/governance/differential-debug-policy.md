# Differential Debug Policy

## Purpose

Differential debugging shortens investigation of virtual-hardware and DOS
divergences. It is a development-only verification method, not a compatibility
backend, runtime dependency, or release feature.

## Boundary

ntvdm64 builds a project-owned optional trace interface. Machine and DOS code
emit normalized events to a null-by-default trace sink. A reference adapter,
including one that drives a locally installed NTVDMx64 debugger, runs out of
process under `tools/research/differential/` and is never linked into ntvdm64.
The optional Bochx/Bochs paired-step CPU bridge is a distinct local research
tool under `tools/research/bochx/`: its coupling exists only inside a separate
developer reference build and never in an ntvdm64 or nxvm product process.

NTVDMx64 can provide evidence for bounded COM and DOS ABI probes. Bochx/Bochs
can provide evidence for bounded CPU instruction/state probes. Neither is an
oracle for NXVM POST, BIOS, virtual disk boot, PIC/PIT, device initialization,
or platform behavior. Those M1 behaviors compare against the preserved NXVM
baseline and focused project-owned observations.

## Required Trace Contract

The project-owned interface is versioned and append-only. A trace session
records source and fixture identities, build identity, event-schema version,
and deterministic input seed. It supports these event classes:

- checkpoint: sequence number, linear PC, selected registers, masked FLAGS;
- CPU control: step boundary, interrupt, return, halt, reset, and stop reason;
- virtual hardware: I/O-port access, IRQ, DMA when used, disk request/result,
  text-display update, and device reset;
- DOS: loader transition, approved `INT 21h` service, handle result, fixture
  filesystem result, deterministic Console input/output, and guest exit.

The initial C interface is a project-owned `ntvdm64_trace_sink` callback table.
`NTVDM64_ENABLE_TRACE=ON` enables it only for a verification build; ordinary
and release builds omit its instrumentation. The concrete wire encoding is a
test-tool decision, not a core ABI.

## Comparison Method

Compare at explicit checkpoints, not unbounded instruction lockstep. Lockstep
is permitted only for deterministic microprobes with an instruction budget.
Each experiment declares register and FLAGS masks, memory ranges, expected
device events, and tolerated implementation-specific differences.

Every differential runner enforces all of these:

- instruction budget;
- wall-clock timeout;
- no-progress limit based on checkpoint/event advance;
- maximum raw-trace byte budget with a unique ignored output path;
- bounded ring buffer of the last trace events; and
- separate-process-tree termination and a divergence report on failure.

The runner monitors raw trace growth independently of guest progress. At any
instruction, time, no-progress, or byte limit it terminates all owned child
processes, waits for their trace handles to close, records only the final size
and compact checkpoint evidence, then deletes the raw trace by default. A
timeout without process-tree cleanup and byte containment is not a bounded
experiment. The shared operational rules, including required free workspace,
are in `docs/planning/execution-policy.md`.

## Cleanup Gate

When a bounded experiment reaches its verdict, remove its experiment-specific
direct hooks, reference configuration, and raw reference trace capture from the
active build. A reusable Bochx bridge may remain only as an optional local
research tool; retain a project-owned neutral trace sink or probe only when it
has a focused regression test and no external reference dependency. No M1 or M5
milestone closes with an enabled external-reference bridge in the default build,
runtime, test fixture, or release package.
