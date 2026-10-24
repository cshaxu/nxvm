# T570 S1 — PC110 Storage And Flash Source Gate

## Scope and method

This audit compares the existing MyPC110 composition and the reusable X86-NXVM
media path with the current, read-only Open-Source-PC110, PC110-EMU and
PC110-QEMU research trees.  It imports no source, firmware, media or ROM
bytes, and adds no guessed I/O route.

## Disposition ledger

| Candidate | Source-qualified facts | Current owner | S1 disposition |
| --- | --- | --- | --- |
| System flash read mapping | MyPC110 already binds the selected immutable BIOS through the single Core firmware-provider route. Open-Source-PC110 identifies the system BIOS flash and its separate write-enable/extra-address wiring. | `app-mypc110/pc110_firmware.c` plus Core immutable-ROM mapping. | Retain the existing read-only mapping. Flash commands, bank selection and write-enable/VPP protocol are not admitted: the source does not establish their CPU-visible register transaction, erase/program completion or reset/error behavior. |
| CompactFlash / ATA storage | Open-Source-PC110 identifies CF as mass storage on the ISA-side topology and records its 16-bit data path. | No MyPC110 CPU route or media owner exists. | Not admitted. A physical CF connection does not prove an ATA base/control decode, byte-lane steering, reset, IRQ, DMA or removal contract. |
| RB5C396 PCMCIA sockets | Open-Source-PC110 identifies the Ricoh RB5C396 and two PC Card sockets; its motherboard reconstruction establishes slot power and card-detect connectivity. | No MyPC110 controller exists. | Not admitted. No complete CPU-facing ExCA register mapping, socket-window mapping, card-detect/IRQ semantics, reset or error contract has been established. |
| Dock floppy | Open-Source-PC110 identifies a FDC37C665IR and documents Pluto/Bowman floppy signals to the dock. | Existing X86-NXVM FDC mechanisms are PC/AT board mechanisms, not a PC110 route. | Not admitted. The evidence does not define the PC110 Super-I/O configuration, decode enable, IRQ/DMA wiring, dock-presence and removal/reset semantics. |
| PC110-EMU storage ports | PC110-EMU registers `1F0h–1F7h`, `3F2h–3F7h`, `3F6h` and `3E0h–3E1h`, but its handlers explicitly return `FFh` or are named `placeholder_write`. | External L2 research only. | Rejected as a production contract: it is explicit scaffold code, not device behavior. |
| PC110-QEMU boot path | PC110-QEMU documents a software boot completer / interrupt path. | External L2 research only. | Rejected: host-side boot completion would violate the real-firmware and single-device-owner rules. |

## Ownership and boundary review

The current `mypc110_base_composition_create()` has one immutable firmware
provider plus App-local configuration, font and F65535 routes.  It deliberately
does not instantiate PC/AT FDC/HDC/media machinery.  The reusable
`x86-nxvm/machine/media` path is therefore not a drop-in PC110 boot solution:
attaching it before the PC110 controller topology is known would silently turn
the PC110 into a PC/AT-shaped machine, invent a first floppy and conceal
card/removal failure behavior.

Likewise, mapping writes into the external BIOS asset would violate the BYOB
asset boundary.  An App-local write reject without a real flash command route
would only manufacture a second, misleading firmware owner.  The existing
immutable provider is the sole correct behavior for the presently admitted
facts.

## Result and transfer

S1 admits no production route or test because every unresolved row lacks a
complete CPU-visible contract.  This is a bounded negative result, not a claim
that PC110 storage is unsupported forever.

The real-firmware POST/setup/guest-boot receiver owns the next evidence
threshold.  It must first execute the selected legal firmware and capture the
actual controller configuration, ATA/PCMCIA/FDC decode, reset, IRQ/DMA,
removal and failure transactions.  Only then may a follow-up receiver admit a
single App-local controller and connect it to the existing media mechanisms.
The audio/peripheral receiver remains independent.

