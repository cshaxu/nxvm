# M5 T32 Mapper 3 / CNROM Design

The [NESdev CNROM reference](https://www.nesdev.org/wiki/CNROM) defines the
finite Mapper-3 board selected by T30: fixed 32 KiB PRG-ROM at `$8000-$FFFF`,
a switchable 8 KiB CHR-ROM window at `$0000-$1FFF`, fixed header-selected H/V
mirroring, and an `$8000-$FFFF` latch. Original hardware has AND bus
conflicts; T30 rejects NES 2.0 submappers, so T32 adopts that finite legacy
AND policy just as T31 does for UxROM.

Current ownership needs no cross-component change. Cartridge owns CPU/PPU
mapping and CIRAM policy; Bus only forwards high CPU addresses; PPU only
forwards pattern access. CNROM's nonzero CHR declaration already makes
`chr_ram` false, so direct PPU writes are rejected by the existing neutral
path.

## Fixed Contract

- Only T30's iNES Mapper-3 shape is enabled: 2 PRG banks and 1--4 CHR banks.
- Reset/create selects CHR bank 0. CPU PRG reads remain entirely unbanked.
- A write at `$8000-$FFFF` latches `(cpu_value & current_prg_rom_byte) %`
  `chr_bank_count`; writes below `$8000` cannot change it.
- PPU reads map the selected full 8 KiB CHR bank. PPU writes remain false
  because CNROM is CHR-ROM.
- The header mirroring relation remains fixed across bank writes.
- Mapper 185 security diodes, oversize CNROM, PRG-RAM, no-conflict variants,
  IRQ/audio and NES 2.0 submappers remain rejected/excluded.

## S Sequence

| S | Outcome |
| --- | --- |
| S1 | Authority/ownership audit and finite contract design. |
| S2 | Add Cartridge's Mapper-3 latch/read behavior and a dedicated synthetic fixture proving every 1--4 CHR bank, reset, wrap, AND conflict, write protection and fixed H/V mirroring. |
| S3 | Add media/driver lifecycle coverage, complete x64/x86 regression and delivery evidence. CNROM has no supplied commercial input, so fixture qualification is explicitly separate from final six-input acceptance. |
| S4 | Independent closure audit; any gap receives a corrective S before T33 admission. |

The dedicated fixture uses bank-labelled PRG and CHR data so it can prove both
the current ROM byte used for conflict resolution and every CHR bank without
using an owner asset.
