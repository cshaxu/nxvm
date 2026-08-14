# M5 PC/AT NMI Source Ownership

## Purpose

Close the selected IBM PC/AT NMI-source ownership gap identified by T350 S4.
The task first chooses one documented physical source, then implements its
assertion, latch/clear, mask, reset, timeline, and CPU-delivery contract
through the existing NMI owner.

## Scope

The candidate depends on the PC/AT device/chip/port/bus completeness audit.
It requires the selected IBM/Intel hardware contract, profile wiring, port or
device source state, deterministic event visibility, real/protected/ordinary
VM86 delivery proof, reset/finalize behavior, and a source/consumer sweep.

## Boundaries and completion standard

Do not synthesize RTC NMI, treat port `61h` as a substitute, add generic NMI
delivery machinery, expose host faults, or claim parity hardware that has not
been selected. The selected source must have one producer and one clear path;
unselected NMI sources remain explicit debt. This candidate precedes
bus-timed/cycle-exact work because those tasks need a truthful interrupt-source
set, but it does not itself allocate physical bus cycles.
