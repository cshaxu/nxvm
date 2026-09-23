# M4 T21: MMC1 Cartridge Profile

T21 delivers the bounded no-trainer/no-battery MMC1 profile required by the
owner-local Dr. Mario input. Cartridge parsing now distinguishes mapper 0 and
the admitted mapper 1 header; the latter accepts exactly 32 KiB PRG-ROM and
32 KiB CHR-ROM. Media intake is correspondingly bounded at 65552 bytes.

MMC1 owns five low-bit-first serial writes, reset-to-fixed-last-bank behavior,
PRG/CHR selection and control-selected single-screen, vertical and horizontal
CIRAM layouts. CPU writes reach it through the production cartridge bus, and
PPU CHR/CIRAM accesses use the selected mapping. Mapper-0 behavior remains
covered by its existing contracts.

`mynes.core.mapper1-smoke` proves the production CPU-bus and PPU-register path;
the cartridge contract expands header, banking and mirror cases. The optional
owner-local load observation reads the ignored Dr. Mario input only when
`MYNES_OWNER_MMC1_ROM` is supplied; x64 and x86 both accepted its actual header
and contents without copying or committing bytes. Full x64/x86 JUnit suites
each report 104 tests, zero failures and zero disabled tests.

This task does not claim that Dr. Mario is playable: it reaches unsupported APU
register writes before its game flow, so APU/audio and final supported-game
acceptance retain that claim. No Lib/Common source changed.
