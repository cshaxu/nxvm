# M5 T485 IBM PC/XT 5160-268 Board And Device Phase Timing

T485 consumes T484's closed selected-device functional routes.  It may only
connect primary-source board/device timing inputs to the one Core time axis;
it must not create an XT scheduler, AT alias, guessed timing value or host-time
writer.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: every selected timing path has one source-labelled fact, current Core receiver and explicit L2/blocking disposition; only PIT has an immediately eligible rational-clock receiver. [Evidence](../etc/evidence/t485-s1-xt-board-phase-source-ledger.md). |
| S2 | Accepted: IBM's `1/4` PIT relation requires the 4.77 MHz XT board-clock axis; current instruction ticks cannot receive it. [Evidence](../etc/evidence/t485-s2-xt-pit-common-axis-boundary.md). |
| S3 | In implementation: source-map 8088 successful retirement, normal/five-clock external cycles and waits to one Core-owned common-axis receiver, or retain each missing relation as an explicit block. |
