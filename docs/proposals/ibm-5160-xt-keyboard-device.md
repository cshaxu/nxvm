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
3. A bounded IBM XT BYOB boot probe identifies its first actual startup owner.
4. A host-waited repeat of the probe provides a valid semantic terminal before
   any successor repair is selected.
5. A 60-second non-checkpoint is an owner-level startup defect, not a
   performance allowance; existing Core observation classifies its finite
   owner batch before repair.

## Constraints

No 8042 alias, profile-side keyboard emulator, VM/host BAT injection, second
queue, second scheduler, firmware/media import or physical wall-clock claim is
permitted. The device's only downstream operation is completed-byte delivery
to the existing PPI receiver.

## Bootability Completion Boundary

The owner expanded T496 to make the selected IBM 5160 route genuinely bootable
to a DOS semantic checkpoint. This is an integration completion boundary, not
a replacement for the completed independent controller tasks. It consumes the
finite boot chain: reset/ROM/8088 and mapped RAM; PIT0-to-PIC IRQ0; PIT1-to-DMA0
refresh; PPI/XT keyboard; 8272A-to-DMA2-to-IRQ6 floppy transfer; CGA snapshot
publication; and the scheduler/interrupt hand-off joining those owners.

The IBM 5160 Technical Reference and the selected Intel chip manuals remain
normative. Existing unit ledgers supply their source-backed rows. 86Box, MAME,
Bochs, PCjs and QEMU may expose an omitted integration hypothesis, but neither
their code nor a firmware workaround is imported or treated as specification.
Every repair S first consumes its complete implicated batch, changes its
existing sole owner, deletes an obsolete route where one exists, and proves the
same semantic checkpoint with the admitted external media corpus. A 60-second
Turbo non-checkpoint is a defect, never a performance allowance.
