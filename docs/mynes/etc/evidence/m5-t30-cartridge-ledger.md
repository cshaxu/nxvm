# M5 T30 Cartridge Format And Profile Ledger

## Current-State Audit

`core_cartridge_create` currently decodes the iNES header directly and accepts
only three game-size profiles. `core_media` separately limits the raw file to
the M4 Super Mario Bros. 2 size and removes a 127/128-byte title suffix. The
machine already owns the cartridge as one complete object, while media owns a
temporary read buffer. T30 preserves those ownership boundaries.

## S Sequence

| S | Outcome | Estimated tracked surface |
| --- | --- | --- |
| S1 | Immutable descriptor/profile design and complete acceptance ledger. | Documentation only, about 2 files. |
| S2 | Parse/validate iNES metadata, build one descriptor, select an admitted profile, and use its maximum payload at the media boundary. | `src/core/cartridge*`, `src/core/media.c`, Core tests, manifests; about 6-9 files. |
| S3 | Exhaustive profile/header/size negative tests and both-architecture regression, including all supplied-header shapes without recording ROM bytes. | Core tests/CMake/manifests and artifacts; about 3-6 files. |
| S4 | Independent T30 audit, documentation and closure. Any unresolved profile or lifecycle defect receives a new corrective S. | Evidence/state only unless a defect is found. |

## Descriptor Contract

Core gains one immutable internal descriptor produced before allocation/copy.
It contains format revision, mapper and submapper identity, PRG/CHR byte
counts, CHR-RAM selection, header mirroring, trainer/battery/RAM declarations,
and the selected finite profile. Arithmetic validates every multiplication and
addition before pointer formation. Cartridge construction copies only the exact
logical payload selected by the descriptor.

Media computes the maximum from the same declared profile capacity plus the
only permitted legacy suffix. It never decides mapper behavior. It passes an
exact normalized payload to Core; unsupported header/profile/length leaves the
existing machine published by the caller unchanged.

## T30 Admission Ledger

| Profile | iNES 1 admission after S2 | M5 receiver | Current supplied header disposition |
| --- | --- | --- | --- |
| NROM | Mapper 0, 1-2 PRG units, 0-1 CHR units, fixed H/V mirroring | retained M4 | Super Mario Bros.; accepted. |
| MMC1 | Mapper 1, 2 PRG units, 4 CHR units, fixed M4 no-battery board | retained M4 | Dr. Mario; accepted. |
| UxROM | Mapper 2, 8 PRG units, zero CHR units, fixed H/V mirroring | Mapper 2 | Jackal; descriptor accepted, execution waits for Mapper 2. |
| CNROM | Mapper 3, 2 PRG units, 1-4 CHR units, fixed H/V mirroring | Mapper 3 | No supplied input; project fixtures only. |
| MMC3 | Mapper 4, 2-32 PRG units, 0-32 CHR units, standard non-battery MMC3 | Mapper 4 | Super Mario Bros. 2, Super Mario Bros. 3 and TMNT III; descriptor accepted, extended execution waits for Mapper 4. |

T30 rejects trainer, four-screen, battery-backed or declared PRG-RAM profiles,
NES 2.0 headers, unknown submappers, unsupported mapper IDs and nonzero legacy
padding. Battery/RAM fields are retained in the descriptor for the later
persistence receiver; accepting them is not deferred behavior. Mapper 004
MMC6 and NES 2.0 submapper variants remain explicit format rejections until the
Mapper-4 package adds direct behavior and tests.

## Test Ledger

S2/S3 must prove magic, iNES-versus-NES-2.0 discrimination, mapper composition,
all accepted PRG/CHR edges, exact payload and suffix sizes, overflow/maximum
rejection, trainer/four-screen/battery/RAM rejection, profile identity and
transactional media replacement. Existing NROM/MMC1/MMC3 coverage migrates to
the descriptor-facing fixture builder; no test reads an owner ROM.

## Similar-Issue Disposition

The header validation is concentrated in `core_cartridge_create`; raw-size
normalization is concentrated in `core_media`. No App/Common/Lib parser or
parallel cartridge path exists. Mapper behavior remains in Cartridge and is
intentionally deferred to the next three candidate tasks.
