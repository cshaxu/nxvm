# T580 S3 — TFT firmware C000 video-ROM status gate

## Scope

This record classifies the next concrete receiver reached by the real,
external TFT PC110 firmware.  It does not claim successful POST, display,
Setup, storage, DOS, or a publishable PC110 artifact.

## Probe

The existing `mypc110-real-firmware-post-diagnostic` was temporarily run with
its normal executor, no firmware interception, and a 100,000,000-instruction
budget on both x64 and x86.  The admitted TFT ROM/font inputs remained
external.  Both widths produced the same material result; the x64 result was:

```text
status=0 reason=1 executed=100000199 resets=1 waits=0 advanced=0
c000=0/1842524 e000=0/0 port61-wait=1/1
state=c700:00000000
leave-rom=f000:00003f67 ... -> c000:00000003 eb 45 90 30
```

The output also reports no Core fault.  The diagnostic's exception-delivery
counter is not treated as a hardware-fault count; it contains ordinary
delivered guest interrupt/software-service activity and is not a licence to
infer a missing interrupt controller.

## Receiver disposition

The PC110-QEMU research implementation describes the identical BIOS transfer
at `F000:3F67` / `F000:4293` as a far call to the C&T VGA BIOS at `C000:0003`
(`PC110-QEMU`, `qemu/hw-misc/pc110-chipset.c`, comments near the `vgac000`
mapping).  It separately models the VGA input-status register at `3DAh` with
a changing vertical-retrace bit (`PC110-EMU`,
`Sources/PC110Core/pc110_core.c`, `f65535_vga_read`).  These are research
references only: no code is imported.

NXVM's construction initially selects the same video bank, but the trace
proves that an EC/ED writable-shadow transition later lets read lookup fall
through to raw bank-zero bytes: C000 reports `55 AA 00 EB`, whereas the
selected C&T bank is `55 AA 47 EB`.  It then executes those wrong bytes after
relocation and repeatedly delivers invalid-opcode exceptions.  This is the
immediate App-local firmware-map defect.  The existing F65535 implementation
also resets the attribute flip-flop at `3DAh` but returns zero
unconditionally; that is the adjacent, finite chip-level L2 behavior, not
the evidence-backed explanation for the current malformed C000 path.

The next S may retain the selected C000 bank for reads while independently
controlling writes, and implement only a controller-local L2 changing vertical
retrace bit while retaining the existing flip-flop reset.  It must not add a
renderer, host clock, BIOS-address exception, QEMU-style POST interception,
or a product-release claim.  The period and exact panel timing remain
unqualified.
