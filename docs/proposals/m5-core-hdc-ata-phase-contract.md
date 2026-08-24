# M5 Core HDC ATA Phase Contract

## Purpose

Define one selected ATA/IDE PIO controller and logical-media phase contract.

## Admission And Dependencies

This candidate is blocked until an ATA/IDE original specification and selected
controller/backing personality are admitted under the source policy. It then
consumes PIC and T449; it does not consume or represent IBM MFM/ST-506.

## Scope And Completion

After admission, freeze reset, BSY/DRQ, command/data/error/result, IRQ14,
IDENTIFY, read/write, cancellation and logical-media detach rows. Prove no
partial command or host-latency timing leak.

## Boundary

MFM/ST-506, Read/Write Long, ECC and physical-sector semantics remain their
explicit TODO receivers. No ATA-like fallback may be presented as Model-339
fixed-disk support.
