# M5 T33 S5 - MMC3 PRG-RAM Format Correction

S5 resolves S4's descriptor finding without broadening the product format
contract. iNES Mapper 4 now accepts header byte 8 values zero and one. Value
one declares the standard non-battery 8 KiB PRG-RAM already owned by the MMC3
Cartridge receiver. The RAM remains volatile; persistence is still T34 work.

The exception is deliberately mapper-specific. Byte 8 values above one reject,
as do a battery flag, trainer, four-screen layout, NES 2.0 and every non-Mapper
4 profile with a nonzero byte 8.

The existing NROM all-header-byte rejection loop directly covers the
non-Mapper-4 boundary. The expanded Mapper-4 fixture directly covers byte 8
equals one accepted, byte 8 equals two rejected, and the battery flag rejected.
Focused x64 and x86 runs both passed `mynes.core.cartridge-contract-smoke` and
`mynes.core.mapper4-smoke`. Both versioned product executables were rebuilt.
