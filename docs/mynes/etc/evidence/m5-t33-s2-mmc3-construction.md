# M5 T33 S2 - MMC3 Construction Evidence

S2 expands the finite standard iNES Mapper-004 receiver from the earlier
SMB2-sized 8 PRG/16 CHR shape to the documented 2--32 PRG and 0--32 CHR-bank
range. A zero-CHR descriptor owns 8 KiB CHR-RAM; all other admitted sizes own
their declared CHR-ROM bytes.

Mapper 4 owns its 8 KiB volatile PRG-RAM. `$A001` bit 7 enables the `$6000`
through `$7FFF` window and bit 6 prevents writes; Bus only forwards that window
for Mapper 4. The RAM stays transient until T34 defines persistence.

MMC3 CHR-RAM reads and writes now use the same active MMC3 bank-register
translation. This fixes the initial, all-zero register state and every later
inversion/bank selection: a PPU write is observable through the same mapped
address that a PPU read uses.

Direct fixture proof covers all four capacity corners `(2,0)`, `(2,32)`,
`(32,0)` and `(32,32)`, PRG-RAM enable/write protection/disable, and both ends
of an 8 KiB CHR-RAM pattern table. The pre-existing fixture retains all eight
bank registers, PRG mode, CHR inversion, fixed banks, mirroring and qualified
A12 IRQ coverage.

Focused x64 and x86 runs passed on 2026-09-22:

- `mynes.core.cartridge-contract-smoke`
- `mynes.core.mapper4-smoke`

Both product architectures were rebuilt into `assets/binary/`. S3 owns the
full regression and optional ignored owner-ROM lifecycle probes.
