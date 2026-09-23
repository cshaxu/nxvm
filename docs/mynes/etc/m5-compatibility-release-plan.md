# M5 Compatibility And Release Plan

## Fixed Outcome

M5 extends MyNes from M4's two game-specific cartridge profiles to a finite six-input acceptance set. Every supplied iNES input must load, start, publish Window and Core-generated Console frames, accept relevant controls, run without a Core trap for its bounded scenario, and cleanly pause, stop, eject and exit on x64 and x86. ROM bytes and local paths remain unrecorded.

| Input role | Header profile | Receiver |
| --- | --- | --- |
| Dr. Mario | Mapper 1, 32 KiB PRG, 32 KiB CHR | Existing M4 regression retained in final acceptance. |
| Super Mario Bros. | Mapper 0, 32 KiB PRG, 8 KiB CHR | Existing owner-accepted NROM regression retained in final acceptance. |
| Super Mario Bros. 2 | Mapper 4, 128 KiB PRG, 128 KiB CHR plus legacy display suffix | Existing M4 regression retained in final acceptance. |
| Jackal | Mapper 2, 128 KiB PRG, CHR RAM | Mapper-2 package. |
| Super Mario Bros. 3 | Mapper 4, 256 KiB PRG, 128 KiB CHR | Mapper-4 package. |
| TMNT III | Mapper 4, 256 KiB PRG, 256 KiB CHR | Mapper-4 package. |

## Mapper Profiles

Mapper 2 is UxROM: one switchable 16 KiB PRG bank, one fixed final bank, fixed header-selected mirroring, 8 KiB CHR RAM, and explicit bus-conflict policy. Mapper 3 is CNROM: fixed 32 KiB PRG, switchable 8 KiB CHR bank, fixed header-selected mirroring, and explicit bus-conflict policy. Both policies use the NES 2.0 submapper distinction where available; legacy iNES ambiguity has a documented default and fixture coverage.

"Complete Mapper 4" means the iNES Mapper 004 standard MMC3 profile: variable PRG ROM through 512 KiB; variable CHR ROM through 256 KiB or the documented CHR-RAM profile; 8 KiB optional PRG RAM; RAM enable/write protection; fixed or register-selected horizontal/vertical mirroring; all bank modes; the complete register-pair protocol; and scanline IRQ behavior driven by qualified PPU A12 edges. NES 2.0 distinguishes MMC3, MMC6 and other Mapper 004 variants. The Mapper-4 candidate must either implement each admitted submapper with direct proof or reject it explicitly at format admission; it cannot silently execute an ambiguous variant as standard MMC3.

Mapper 004 variants outside the declared ledger, mapper 118/119, multicart derivatives, PAL/Dendy timing, expansion audio, additional controllers, save states, replay, rewind and networking remain excluded.

## Ordered Candidate Packages

1. Cartridge format and profile ledger.
2. Mapper 2 UxROM.
3. Mapper 3 CNROM.
4. Complete profiled Mapper 4.
5. Persistence, configuration and reproducible release baseline.
6. Six-input product acceptance and M5 closure audit.

The first package defines a stable descriptor and size/format admission before any mapper is generalized. Mapper 2 and Mapper 3 may execute independently after it. Mapper 4 depends on it and owns the two previously rejected supplied Mapper-4 inputs. Persistence/release depends on descriptor identity; final acceptance consumes every prior package.

## Authoritative Basis

The [hardware reference register](hardware-references.md) remains the local authority index. Planning additionally consulted NESdev's MMC3, UxROM, CNROM, iNES and NES 2.0 submapper documentation. The MMC3 register layout, capacity, PRG-RAM and variant distinction are recorded by [NESdev MMC3](https://www.nesdev.org/wiki/MMC3); the Mapper 2/3 bus-conflict distinction is recorded by [NESdev submappers](https://www.nesdev.org/wiki/NES_2.0_submappers).

## Release Baseline

Persistence uses ROM content identity plus mapper/profile and format version. Core owns battery RAM bytes and dirty state; App selects paused save/load boundaries; Lib owns generic atomic replacement. The release package retains the versioned x64/x86 EXEs and sole editable `mynes.ini`, adds a generated manifest with hashes and toolchain/version identity, and proves rebuild reproducibility to the defined release criterion.
