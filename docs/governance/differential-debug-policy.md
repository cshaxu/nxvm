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

NTVDMx64 can provide evidence for bounded COM and DOS ABI probes. It is not an
oracle for NXVM POST, BIOS, virtual disk boot, PIC/PIT, or device initialization.
Those M1 behaviors compare against the preserved NXVM baseline and focused
project-owned observations.

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
- bounded ring buffer of the last trace events; and
- separate-process termination and a divergence report on failure.

## Cleanup Gate

When a bounded experiment reaches its verdict, remove its temporary bridge,
reference-specific configuration, reference trace capture, and diagnostic-only
hooks from the active build. Retain a project-owned neutral trace sink or probe
only when it has a focused regression test and no external reference dependency.
No M1 or M2 milestone closes with an enabled external-reference bridge in the
default build, runtime, test fixture, or release package.
