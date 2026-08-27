# IBM 5160 8255 PPI, Keyboard And NMI Phase Contract

## Purpose

Complete the XT PPI/keyboard/parity-I/O-check/NMI unit at its one Core owner;
it is not an AT 8042 task.

## Required sequence

S1 admits original 8255 and IBM XT references. S2 freezes List 1 for port
bits, keyboard input, parity/I-O check, NMI edge, reset and timing/order.
S3 maps List 2 to the present PPI/NMI owner and all producers/consumers.
Implementation closes the complete batch with reset and IRQ/NMI proof.

## Boundaries

No 8042 command set, host input shortcut, VM guest-state mutation or second
NMI delivery route.
