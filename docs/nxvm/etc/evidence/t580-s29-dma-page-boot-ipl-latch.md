# T580 S29: PC110 DMA-page / Boot/IPL latch boundary

## Scope

This change corrects the board-owned meaning of I/O ports `80h` through `8Fh`
for the PC110 personality.  It does not claim a complete boot receiver,
choose a Boot/IPL reset value, alter the admitted external firmware, or modify
Lib, Emulator, or X86 Core.

## Evidence and model level

The selected TFT BIOS (`pc110-tft-bios-1995-09-19.rom`, external BYOB asset)
contains the reachable sequence at `F000:52B3`:

```text
mov al,6Eh
call F000:55AD
xor al,al
out 8Bh,al
sti
int 19h
```

`Open-Source-PC110` commit `02ccf2e8e19b298a2cf04c5d430278eb087cc368`,
`Discovery/Chipset/readme.md`, identifies `8Bh` as the repurposed Boot/IPL
latch rather than a PC/AT DMA page register.  The independent behavioral
references `PC110-EMU` commit `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c`
(`Sources/PC110Core/pc110_core.c`) and `PC110-QEMU` commit
`bae390ebf21665ec97860d28e26ec46f1fe8f4e9`
(`qemu/hw-misc/pc110-chipset.c`) both implement `80h` through `8Fh` as a
zero-reset byte file.  They are read-only L2 behavior references; no source
was copied.

Accordingly, the PC110 Board-Base configuration selects one 16-byte,
zero-reset scratch file for the whole `80h`–`8Fh` range.  Conventional boards
retain their existing sparse primary/secondary DMA page mapping.  The second
8237 control register range (`C0h`–`DEh`) remains installed for both
personalities; it is not part of the latch substitution.

## Verification

`x86-nxvm.board-base/dma_ports` covers one-controller and two-controller
normal mappings plus the two-controller scratch personality.  It verifies
that all sixteen scratch bytes round-trip and reset to zero; the ordinary
secondary DMA page mapping remains covered only by its normal personality.

`mypc110.unit/base-composition` exercises the actual PC110 composition:
every port `80h`–`8Fh` starts at zero, preserves an independent byte, and
returns to zero after Core reset.

Both tests pass on GCC x64 and x86.  The existing admitted-firmware diagnostic
also passes on both widths through the post-C000 port-61 return checkpoint;
that confirms prior execution reaches `F000:EA84` but does not claim that the
later `out 8Bh` has yet executed.  The static BIOS target plus isolated Board
contract are the present evidence for this receiver boundary.

## Disposition

The model is a bounded L2 board personality.  Future work may observe the
actual `8Bh` write once firmware execution reaches it, but must not infer boot
success from a retirement ceiling or invent downstream IPL behavior.
