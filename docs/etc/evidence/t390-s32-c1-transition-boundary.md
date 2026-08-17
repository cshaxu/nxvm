# T390 S32: C1 Transition Boundary

## State machine

The existing test-only Model-40 retirement capture now records only copied
aggregate state.  C0 completes on the first successful protected-to-real
transition after reset.  The candidate C1 boundary is the first later
successful real-to-protected transition.  The tracker retains neither address,
bytes, ROM/media identity, path, hash, provenance, trace, nor raw guest state.
Its synthetic regression proves the ordered real, protected, returned-real,
and later-protected sequence reaches both boundaries.

## Contained replay

One owner-managed external-asset replay ran with the existing two-million
retirement containment limit and a forty-five-second wall-clock cap.  It
completed within the wall-clock bound with 2,000,000 classified successful
retirements, zero source-unallocated observations, 104 aggregate forms,
protected mode observed, and C0 observed.  The later real-to-protected C1
candidate was not observed before containment exhausted.

This is a complete bounded absence for this candidate checkpoint, not proof
that all later C1 behavior is absent and not a timing, board-clock, physical
retirement, or L3 conclusion.

## Similar-issue sweep

The existing capture has one phase/checkpoint owner.  Its synthetic mode and
external replay share that owner; no production observer, VM profile, Core
interface, or separate raw-output phase tracker was added.  The existing C0
flag now retains its original meaning and the C1 candidate is explicit rather
than inferred from a retirement count.

## Transfer

A later T390 receiver must choose a different finite C1 semantic checkpoint
from already copied state or add only the minimum bounded copied scalar state
needed to express one.  It must not use a retirement or wall-clock budget as a
checkpoint, retain raw observation data, assign a timing value, or advance
DeskPro board/L3 work.

## Coordinator acceptance

The coordinator reviewed pushed P1 47352d34 against the admitted S32 packet and original request. The test-only capture holds one phase owner, publishes only aggregate state, and neither expands a production interface nor changes CPU/profile behavior. The synthetic transition proof, contained replay cleanup, focused CTest target, documentation governance, and diff hygiene pass. S32 is accepted; the specific second protected-entry candidate transfers as absent within containment.
