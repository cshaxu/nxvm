# Board Bus, Interrupts And COP

## Objective And Boundary

Implement the frozen board's mapped memory/ROM banks, RAM, interrupts, input
registers, protection/COP operations and sound communication in MyRaidenII Core.
Requires approved V30 execution and the source/set ledger. No IBM PC board,
BIOS, port-map shortcut or universal arcade device framework is introduced.

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
