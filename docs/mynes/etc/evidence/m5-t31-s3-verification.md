# M5 T31 S3 UxROM Integration Verification

## Integration Coverage

`mynes.core.media-failure-contract-smoke` now replaces its accepted NROM
machine with a synthetic, finite Mapper-2 medium. Its fixed final PRG bank
contains a reset-vector loop. The test runs 64 driver slices after insertion,
asserts no Core trap, then verifies that an invalid medium mode leaves this
newly accepted Mapper-2 machine intact. This proves Mapper-2 construction and
mapping across Driver/Media/Machine lifecycle boundaries without an owner ROM.

`mynes.core.owner-rom-probe` additionally accepts the optional
`MYNES_OWNER_UXROM_ROM` variable. When the owner chooses to set it, the probe
requires mapper 2, produces either graphics or Core-generated text frames,
runs to the configured slice limit without a trap, drains audio on heartbeat
shutdown and prints only aggregate observations. The variable was absent in
this automated run, so it correctly skipped rather than creating a false
commercial-ROM claim.

## Verification

| Scope | Result |
| --- | --- |
| Focused Mapper-2 / lifecycle / optional-probe tests | x64 and x86 passed. |
| Complete CTest suite | x64 113/113 passed in 120.64 seconds. |
| Complete CTest suite | x86 113/113 passed serially in 91.53 seconds. |
| Documentation gates | `-Scope All` and `-SelfTest` passed. |
| Source hygiene | `git diff --check` passed. |

## Current Delivery Artifacts

| Artifact | SHA-256 |
| --- | --- |
| `assets/binary/mynes_0_1_0011_x64.exe` | `6B2884CC3BE0C9F4A3B0459E809AD17769E6788178AE42181B8C6F60E6527938` |
| `assets/binary/mynes_0_1_0011_x86.exe` | `E30AA950A24BFBE503058A140094F9B088DB5BBD6FAA1D5EE57DCF8848AFA06F` |

The executable pair was rebuilt in S2 when production Core mapping changed;
S3 itself changes only test/probe code and this evidence. Owner ROMs remain
ignored and no path or byte is stored in this record.
