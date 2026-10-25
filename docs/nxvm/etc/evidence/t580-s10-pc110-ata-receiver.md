# T580 S10 — PC110 ATA Receiver

## Admitted fact

Open-Source-PC110's `Discovery/Storage/readme.md` records a standard ATA task
file at `1F0h` through `1F7h`, routed on IRQ14, from both live capture and the
selected firmware image. It also records that the BIOS uses CHS operations and
does not issue `IDENTIFY DEVICE`; later software can nevertheless use the ATA
device directly.

This is an App-local L2 receiver. It reuses the existing X86-NXVM ATA PIO
controller and fixed-disk media owner. It does not model the Ricoh PC-Card
controller, attach a disk image, infer a disk geometry, or assert any ATA
mechanical timing on the Core tick axis.

## Change

MyPC110 supplies the documented task-file addresses and IRQ14 to its existing
Board plan, enables its existing fixed-disk owner, and configures the plan with
the existing fixed-disk media ID. The storage initialization path now binds the
existing media registry for either a floppy consumer or an ATA-only consumer.
That condition is generic X86-NXVM construction ownership; it does not alter a
media protocol or add a second route.

`command_ticks` and `next_sector_ticks` remain zero. In the existing HDC
contract that is an immediate owner deadline, not a claimed disk duration.

## Proof and limit

`mypc110.unit/machine-construction` constructs the real App plan with no
floppy slot and checks that the fixed-disk owner is present. Construction would
reject if the ATA topology did not receive the frozen media registry. The
existing X86-NXVM HDC tests retain task-file command/IRQ/media semantics.

The test does not prove that the external TFT firmware has reached ATA, POSTed,
or booted a disk. Firmware and media remain external BYOB inputs, and the
unmodelled PC-Card/power path remains outside this receiver.
