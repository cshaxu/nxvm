# M5 T387: DeskPro 386 Board Bus And Device Timing Closure

## Task Record

T387 consumed the selected Model-40 functional baseline to establish the
source-labelled board/device timing inventory and earliest shared receivers.
It did not accept physical board timing or Model-40 L3: the governing CPU
retirement axis remains mixed-unit until a dedicated shared receiver closes.

## Completed Subtasks

- S1 inventories Model-40 board/device timing sources, current owner graph and
  unknowns.
- S2 rejects an invented CPU/PIT/DCLK scalar.
- S3 defines a bounded exact-model bridge-observation contract.
- S4 qualifies available local secondary references and rejects substitutions.
- S5 reconciles D3PE port-`61h` facts with D4 ownership, and identifies the
  shared successful-retirement physical-timebase blocker.
- S6 independently maps every T387 requirement to evidence or a named receiver
  and closes this task without a physical-time or L3 claim.

## Transfers

The [four-profile CPU physical-timebase closure](M5-T388-four-profile-cpu-physical-timebase-closure-proposal.md)
precedes the [DeskPro board-level timing closure](M5-T396-deskpro-386-board-level-timing-closure-proposal.md), which precedes the [DeskPro physical-device and firmware-timing closure](M5-T397-deskpro-386-physical-device-firmware-timing-closure-proposal.md),
which in turn precedes the [DeskPro Model-L3 audit](../proposals/m5-deskpro-386-l3-audit.md).
The retained [S6 closure audit](../etc/evidence/t387-s6-task-closure-transfer-audit.md)
contains the complete requirement matrix. The original task proposal is retained
beside this record for historical context.