# M5 Bochs Instruction-Differential Debug Bridge

## Purpose

Make difficult CPU/firmware failures reproducible at one instruction boundary
without importing, linking, patching, or distributing Bochs.  An owner-run
external Bochs instance may compare an explicitly selected NXVM Core execution
against the same starting state and report the first semantic divergence.

## Boundary

Core exposes a small debug-only, opt-in harness seam:

- publish a copied pre/post instruction snapshot (CPU architectural state,
  instruction bytes, selected physical reads/writes, and stop/exception
  result);
- accept a caller-controlled instruction budget and an explicit stop request;
- allow a caller to restore only a Core-owned, validated debug snapshot.

The seam has no Bochs type, header, binary, protocol, source text, runtime
dependency, profile branch, device-state mirror, or production scheduler path.
It is inactive unless an external research harness subscribes.  Core remains
the only NXVM state owner; the external adapter merely translates copied
snapshots and compares results.  VM/profile composition supplies the frozen
machine input once and never participates in per-instruction state mutation.

## Evidence And Research Discipline

Intel manuals remain the primary authority.  Bochs is an external behavioural
reference used to locate a disagreement, not an oracle and not a source-import
authority.  Every repaired instruction requires its CPU-profile manual ledger
row and a project-owned regression; a Bochs trace alone cannot establish L3.
Raw recordings stay ignored under a unique `build/` path with instruction,
wall-clock, no-progress and byte limits, process cleanup, and a compact
retained divergence record.  No third-party source, binaries, ROMs, guest
media, machine-local path, trace, or generated comparison output is committed.

## Subtask Plan

1. **S1 - finite bridge contract and provenance audit.** Define the copied
   snapshot schema, supported CPU profiles/modes, memory-I/O observation mask,
   restart/stop semantics, trace budgets and the external-tool license boundary.
   Compare the proposed state set against existing Core debugger and retirement
   observation interfaces; delete duplicate observation routes rather than add
   a parallel recorder.
2. **S2 - Core-owned instruction harness.** Add the smallest debug interface
   that can snapshot, single-step, stop and restore validated Core state while
   preserving the existing instruction execution path.  It must neither expose
   mutable internal pointers nor let an adapter write CPU/device state outside
   the single validated restore operation.
3. **S3 - external Bochs adapter and bounded runner.** Keep adapter code and
   Bochs installation outside the repository.  Provide a documented local
   invocation contract and a project-owned harness that consumes only copied
   Core events, enforces recorder containment, and emits a compact first-
   divergence report.
4. **S4 - CPU differential corpus and closure.** Exercise a finite corpus
   spanning 8086, 8088, 80186, 80286 and 80386 real/protected transitions,
   return/interrupt stack frames, FLAGS, segment reloads, faults and selected
   firmware paths.  Reconcile every divergence with manuals and owner-local
   regression tests; perform the required full unit and integration gates.

## Exit Criteria

- An external Bochs run can compare selected bounded instruction sequences
  against copied Core state and identify the first divergent instruction.
- No Bochs source, patch, executable, ROM, guest input, trace, path, runtime
  dependency or product code enters this repository.
- Core retains one execution/state-owner path; debugger and normal execution
  remain usable with no external tool installed.
- Each accepted CPU repair has manual-ledger evidence and a repository-owned
  regression; the bridge itself is not treated as timing or compatibility proof.
