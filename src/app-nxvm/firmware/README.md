# Project-Owned Default BIOS

This is guest firmware, not a host BIOS service. `nxvm-firmware-build` uses
the shared assembler and Lib file services to produce a 64 KiB ROM in the
ignored build tree. The default product embeds that output; it does not load
an external default BIOS at runtime. Other products embed their external BYOB
ROMs through the same immutable-byte packaging route.

## Recovery Baseline

The owner authorized recovery under the root MIT license in T539 S12.
Source revision: `1cf34c1f452f4e76b7a6e88eccb0953c70ba9cd9`, historical directory
`src/vm/profile/default_profile/firmware/`.

- `bios.h`: boot, video, equipment, memory and system services.
- `fdc_firmware.h`: floppy POST, IRQ and INT 40h services.
- `hdc.h`: INT 13h dispatch and ATA services.
- `qdkeyb.h`: keyboard IRQ and INT 16h services.
- `rtc_firmware.h`: RTC POST, timer IRQ and INT 1Ah services.
- `post_firmware.h`: DMA, PIC and PIT POST.
- `bios.c`: the two keyboard translation tables only; no host BIOS code.

C string assembly was decoded into the adjacent `.asm` files, retaining the
original copyright and instruction style. `entry.asm` and ROM template values
in `build.c` reconstruct the existing owner-built image's guest startup. The
boot routine retains its floppy-to-hard-disk fallback. The old redundant
near jump to the next instruction is omitted; two self-XOR instructions use
the assembler's equivalent register-direction opcode. INT 15h's recovered memory
constant is still 15 MiB; recovery alone does not prove dynamic memory support.

IVT/BDA templates reside in ROM and are copied by guest startup instructions.
The last conventional-memory KiB remains reserved for floppy DMA scratch.
Vectors, templates, code extents and reset entry are assembled by one builder;
there is no runtime mirror of those initialization operations in the host.

The repaired INT 40h uses one SEEK/SIS/READ-ID/data path and one sector-sized
DMA bounce buffer for reads and writes. CMOS drive type selects the transfer
rate and geometry. All controller waits are bounded; result bytes determine
BIOS status and carry. IRQ6 only notifies and acknowledges the PIC; it does
not compete with the service for SIS/results. IRQ14 reads ATA status and
acknowledges both PICs, so a completed hard-disk command cannot retain the
cascade and block later floppy interrupts.

Protocol authorities are Intel 8272A (1982), printed 6-234/235 (SEEK,
RECALIBRATE and status), and IBM PC/AT Technical Reference (March 1984),
printed 5-28/29 and 5-89 (BIOS return contract). These are specification
references, not copied vendor BIOS source. T539 S12 records the complete
firmware regressions and receiver verification; Current owns acceptance. See the
[working evidence](../../../docs/nxvm/etc/evidence/t539-s12-fdc-drive-status.md).
