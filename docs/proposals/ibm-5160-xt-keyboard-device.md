# IBM 5160 XT Keyboard Device

## Objective

Make the XT keyboard a Core-owned device rather than a hidden PPI queue: it
owns keyboard FIFO, serial/reset/BAT state and one composable deadline; PPI
retains only ports, its received-byte latch and IRQ1.

## Source And Scope

IBM's March 1986 XT reference is normative for the selected 101/102-key
keyboard. Its reset/BAT and serial intervals are Manual L3 facts; a selected
deterministic point inside a manual range is macro/L2 integration, not a
physical-time claim. External emulator inspection corroborates separation from
the AT 8042 only and is never imported or normative.

## Subtasks

1. Source work freezes the visual-source ledger, List 1 and List 2.
2. Implementation moves the one keyboard FIFO/input/serial/reset/BAT owner
   into Core and deletes the PPI queue/direct input route.
3. Closure runs owner tests and a bounded IBM XT BYOB boot probe, then
   transfers the next actual startup owner if needed.

## Constraints

No 8042 alias, profile-side keyboard emulator, VM/host BAT injection, second
queue, second scheduler, firmware/media import or physical wall-clock claim is
permitted. The device's only downstream operation is completed-byte delivery
to the existing PPI receiver.
