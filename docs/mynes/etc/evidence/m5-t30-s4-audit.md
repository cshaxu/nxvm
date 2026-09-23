# M5 T30 S4 Closure Audit

## Decision

T30 does not close in S4. The descriptor and media source has one owner,
explicit future-mapper disposition, bounded normalization and transactional
replacement proof. Governance gates pass and the tracked executable pair is
consistent with the verified build. However, the T30 ledger requires direct
proof of every admitted PRG/CHR range boundary. S3 proved only the MMC3 upper
corner (32 PRG / 32 CHR); it did not directly exercise 2/0, 2/32 and 32/0.

## Criterion Review

| Criterion | Evidence | Result |
| --- | --- | --- |
| One immutable Core parser/profile owner | `src/core/cartridge.c` selects all five finite profiles before allocation; `src/core/media.c` only reads and normalizes. | Pass |
| Bounded media input and permitted suffixes | `core_cartridge_maximum_image_bytes` and `core_cartridge_normalize_ines_size`; S3 exact/127/128/invalid-suffix checks. | Pass |
| Explicit runtime disposition | UxROM, CNROM and non-M4-receiver MMC3 profiles return `LIB_STATUS_UNSUPPORTED`; no unsupported mapper can publish a replacement. | Pass |
| Transactional replacement | `mynes.core.media-failure-contract-smoke` retains the accepted machine for storage failure and admitted-but-unimplemented UxROM. | Pass |
| All declared profile edges | NROM/MMC1/UxROM/CNROM edges have direct coverage; MMC3 currently has only the 32/32 upper corner. | **Open** |
| Regression and delivery boundary | x64 and x86 full suites passed 112/112; artifacts are tracked and owner ROMs remain ignored. | Pass |
| Documentation governance | `Verify-DocumentationGovernance.ps1 -Scope All` and `-SelfTest` pass. | Pass |

## Corrective Receiver

S5 adds synthetic MMC3 corner fixtures for 2/0, 2/32 and 32/0. The expected
result remains descriptor admission followed by explicit unsupported execution,
because T30 must not pre-implement T33. It reruns the focused dual-architecture
tests and the governance gate before a replacement closure audit.
