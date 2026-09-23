# M5 T31 UxROM Closure Audit

T31 closes. The audit traced every S1 contract item to the final tree and did
not find a corrective gap.

| Requirement | Evidence | Result |
| --- | --- | --- |
| Finite Mapper-2 admission | T30 admits exactly 8 PRG / 0 CHR Mapper 2; `core_cartridge_create` now constructs it. | Pass |
| CPU windows and reset | `uxrom_prg_bank` defaults to zero; CPU reads map switchable `$8000-$BFFF` and final `$C000-$FFFF`. | Pass |
| Legacy conflict policy | Cartridge, not Bus/PPU, latches `cpu_value & mapped_rom_value`; dedicated fixture proves a conflicting write. | Pass |
| CHR-RAM and mirroring | Direct Cartridge PPU path and both header H/V relationships are exercised by Mapper-2 fixture. | Pass |
| Lifecycle | Synthetic Mapper-2 media replacement runs 64 Driver slices without trap and rejects invalid medium replacement transactionally. | Pass |
| Owner-local boundary | Optional `MYNES_OWNER_UXROM_ROM` probe has no configured input claim and records no ROM path/bytes. | Pass |
| Regression and delivery | x64/x86 each passed 113/113; executable pair is tracked and `assets/roms/jackal.nes` remains ignored. | Pass |
| Governance | All and self-test documentation gates passed; working tree was clean. | Pass |

T31 adds 254 lines and removes 27 across its design, implementation and
verification range. T32 Mapper 3/CNROM is its next, independent receiver.
