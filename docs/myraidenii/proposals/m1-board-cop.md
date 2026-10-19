# Board Bus, Interrupts And COP

## Objective And Boundary

Implement the frozen board's mapped memory/ROM banks, RAM, interrupts, input
registers, protection/COP operations and sound communication in MyRaidenII Core.
Requires approved V30 execution and the source/set ledger. No IBM PC board,
BIOS, port-map shortcut or universal arcade device framework is introduced.

The owner's 2026-10-18 T2 replan assigns the original real-ROM board-path
instruction checkpoints here, after this task implements their actual bus/
COP prerequisites. T2 must first qualify V30 and a ROM-free public Machine/
attachment/vector route; it need not implement this board merely to close CPU
qualification. These real-set checkpoints remain mandatory M1 obligations,
not passed or replaced by synthetic CPU tests. No unfinished CPU form, mode,
bus or timing requirement is transferred from T2.

## Work And Verification

One guest timeline owns CPU/device advancement and stable event ordering.
Test bank boundaries, byte ordering, IRQ/vector acknowledgement, COP/DMA effects,
input edges and sound latches through the real bus interface. Use ROM-free
vectors first, then owner-ROM headless boot checkpoints; unsupported behavior
must be explicit rather than patched game code or scripted return values.

## Exit And Stops

The selected board reaches the ledger's genuine startup checkpoints with
deterministic state and interrupt behavior. Failures/cleanup preserve ownership.
Every COP operation used by the game has an evidenced model and regression.
Unresolved operations are not hidden behind an attract-screen-only closure.
