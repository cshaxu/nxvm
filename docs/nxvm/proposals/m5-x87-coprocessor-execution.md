# M5 Candidate: x87 coprocessor execution

## Objective

Implement and qualify a real, bounded x87 coprocessor personality for the
supported X86 Core CPU profiles.  This replaces the current CPU-side
`WAIT`/`ESC` interface and finite 8087 semantic subset only where an approved
coprocessor model, operation corpus and error/timing contract require it.

## Baseline

The current four PC Apps select `FPU_NONE`.  Core already owns CPU-side
`WAIT`/`ESC`, CR0 `MP`/`EM`/`TS`, `#NM`/`#MF`, optional FPU state and the sole
Core deadline route.  It has a narrow 8087 operation subset and 80287/80387
handoff behavior, but it does not claim complete numeric formats, arithmetic,
environment save/restore, exception delivery or timing.  This candidate does
not retroactively promote that boundary to full x87 support.

The T580 S54 absent-FPU investigation additionally identifies a retained
CPU-side debt for earlier 286/386 profiles: their no-device path consumes
ESC without the generation-specific control-store and pointer behavior
reported by independent hardware research. The queued design audit above
must qualify that complete absence/presence boundary before changing those
profiles; the SX L2 receiver is not silently generalized to them. See
[S54 sources and finite scope](../etc/evidence/t580-s54-post-restoration-route.md).

## Required Design Gate

Before implementation, select the exact first coprocessor personality and
admit a finite operation/context matrix from primary documentation.  The gate
must state:

- CPU/FPU pair and supported product/profile consumer;
- numeric representation, rounding, stack/tag/status/control ownership;
- supported ESC forms, memory formats, environment/state forms and explicitly
  rejected forms;
- masked/unmasked exception, `WAIT`, BUSY/TEST, `#MF` and reset behavior;
- timing evidence grade and the single existing Core deadline owner; and
- test ownership: FPU unit semantics, CPU escape/delivery integration and
  Machine/profile wiring without duplicating one another.

An external mature emulator may inform a labelled L2 model only after the
source policy is followed.  It must not become copied source or a claim of
silicon-accurate timing.  No new executor, second FPU clock, board-private
numeric engine, or App-side arithmetic path is allowed.

## Candidate Work Breakdown

1. Audit current FPU state, CPU escape routing and all profile consumers;
   choose one admissible personality and finite corpus.
2. Complete the selected FPU's state and arithmetic/memory semantics through
   the existing `x86-core/fpu` owner.
3. Connect selected exceptions, `WAIT`/BUSY and deadline behavior through the
   existing CPU and Core Machine paths.
4. Admit a product/profile only after its board wiring and runtime behavior
   are independently verified; otherwise retain `FPU_NONE`.
5. Run dual-width Core/unit and affected integration qualification, rebuild
   only the artifacts whose executable inputs changed, and record remaining
   unselected models/forms explicitly.

## Non-goals

No speculative 8087/80287/80387 completeness claim, generic floating-point
framework, host floating-point shortcut that changes guest semantics, raw
private-machine access, or alteration of the four existing products merely to
make an FPU test runnable.

## Exit

The chosen personality has a source-backed finite contract, one FPU owner,
one CPU/Machine integration path and passing owner-local regressions.  All
unselected models or operation classes remain explicitly deferred rather than
silently accepted.
