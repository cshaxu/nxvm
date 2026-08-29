# M5 Turbo L1 Compatibility Escape

## Purpose

After the active Core event-deadline scheduler convergence task has closed,
make Turbo remain usable when the CPU is blocked by an active internal L1
relation for which no source-qualified deadline exists. This is a bounded,
explicit compatibility escape only. It does not repair the missing timing,
does not promote L1 to L2 or L3, and does not create a second guest clock.

## Dependency And Scope

This candidate follows the active scheduler-convergence packet. It must use
the single Core deadline and event-ordering path produced there; it must not
run in parallel with it or introduce another scheduler.

The frozen coverage universe is every reachable Core state in which all of
the following are true:

- the CPU cannot continue because an internal owner is waiting;
- that owner is explicitly classified L1 and has no source-qualified deadline;
  and
- Core has already considered every known deadline and ordered event that can
  become visible first.

S1 must record each such state in the relevant implementation and gap ledgers,
including its owner, wake condition, reset/cancel path, input/interrupt
interaction, and whether the shared compatibility escape is safe. A state
whose semantics cannot be preserved by this rule remains an explicit blocker;
it is not guessed into a timing model.

## Ownership And Design Constraint

Core remains the sole owner of guest time, CPU wait state, controller state,
and event ordering. Profiles supply only an immutable, validated
construction-time compatibility policy. VM selects Standard or Turbo and
observes copied Core status; it neither computes elapsed guest ticks nor
injects a time advance.

For an eligible L1 wait in Turbo, Core may use that frozen bounded
compatibility quantum only after settling all known deadlines up to the
candidate boundary. It then re-evaluates the complete state before another
step. A known earlier deadline always wins over the compatibility boundary.
Stop, pause, debugger control, reset, guest input, interrupts, and every
source-qualified event must be observed at the normal Core boundary and may
end the sequence immediately.

Standard never invokes this escape. It retains the ordinary blocked/waiting
behaviour and must not turn a host sleep or elapsed host time into guest time.

The work must not add device-specific shims, a generic plugin framework,
controller pointers outside Core, large arbitrary tick batches, or a path that
clears a device wait without passing through its normal state transition.
Later manual evidence, or an external implementation that is demonstrably
consistent with that evidence, may promote a particular owner in its own
controller task; this candidate must keep that distinction visible.

## S Decomposition

1. **Inventory and classification.** Enumerate the complete active L1
   no-deadline universe and its lifecycle/wake rules; correct any ledger
   labels that confuse L1, L2, L3, and external timing evidence.
2. **Core result boundary.** Define the smallest copied Core status needed to
   distinguish an eligible L1 no-deadline wait from a known-deadline wait,
   halt, stop, pause, or input wait. Preserve Core ownership and the existing
   runner authority.
3. **One bounded Turbo path.** Add the single Core-owned compatibility
   progression rule and the frozen profile construction policy. Each step
   must settle known work first and re-evaluate every exit condition.
4. **Mode and lifecycle verification.** Prove that Standard never takes the
   compatibility path and that Turbo preserves debugger, stop/pause, reset,
   input, interrupt, and known-deadline behaviour.
5. **Integration closure.** Exercise the affected ROM/DOS workloads and
   profiles, including the D4-related blocker where applicable; run the
   required gates and record both resolved states and remaining explicit L1
   blockers.

## Exit Criteria

- Every active L1 no-deadline state has a documented disposition.
- Turbo cannot indefinitely spin one Core tick at a time solely because of an
  eligible L1 no-deadline wait.
- No known deadline or ordered event is skipped, reordered, or delayed past a
  compatibility boundary.
- Standard has no compatibility progression and no new host-to-guest time
  injection path.
- The implementation makes no L2/L3 or physical-pacing claim without its
  corresponding source-qualified evidence.
- Focused lifecycle and ROM/DOS regressions, plus the applicable full gates,
  pass with evidence; remaining unsupported owner-specific timing is recorded
  as L1 rather than hidden by the escape.
