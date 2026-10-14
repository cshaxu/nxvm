# Project-Owned Default BIOS

This is guest firmware, not a host BIOS service. `nxvm-firmware-build` uses
the shared assembler and Lib file services to produce a 64 KiB ROM in the
ignored build tree. The default product embeds that output; it does not load
an external default BIOS at runtime. Other products embed their external BYOB
ROMs through the same immutable-byte packaging route.

The builder requires the already validated character-generator input and
normalizes its two eight-row halves into 32-byte glyph slots at ROM offset
A000h. The video-code extent ends before that reserved font area. Mode 03h
loads those bytes through guest CPU writes to plane 2, then configures the
B8000 odd/even character/attribute planes and text stride. There is no host
font initialization or capture-time substitute. Repository-only firmware
tests build the same assembly with a code-owned pattern instead of BYOB bytes.

## Recovery Baseline

The owner authorized recovery under the root MIT license in T539 S12.
Source revision: `1cf34c1f452f4e76b7a6e88eccb0953c70ba9cd9`; the recovered firmware
source now lives in this directory.

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

Text mode 03h initializes 80x25 cells with the complete 16-row font, producing
640x400 pixels. This default-machine custom mode uses the generic EGA register
model, not a claim of an IBM EGA monitor's standard 400-line timing. Vertical
display ends at 399; blanking starts at 424 and ends at 440; retrace spans
425 through 427; the modeled total is 462. These are the former 200-line mode's
vertical endpoints plus 200, preserving its porches. Overflow bits and the
14..15 cursor match the new geometry. Underline remains outside the cell.
IBM EGA Technical Reference printed 64/65 supplies the register facts; the
existing terminal-count and oscillator model remains L2, not hardware-exact
timing. The guest indexed-byte writer owns the port sequence and restores BDA
text geometry on every mode03 call. The live-font, mode-return and ordered
active/retrace/exit receiver uses actual CPU execution, not a host video
default. Other graphics timing and physical oscillator qualification are not
inferred from the text-mode proof. Modes D/E/10 separately initialize their
complete timing registers through the same guest writer and retain the
project's extended, byte-addressed plane layout. The actual mode-return
receiver observes each raster and final text return. Current and the T555 S7
evidence record the still-unqualified Board oscillator/bandwidth consumers.

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
