# Receiving Shared Core absent-FPU correction

NXVM M5 T580 S54 explicitly admits the Core-linked MyRaidenII artifact pair
as a receiving target. Shared P1 `48fe7a544` is confined to 486SX absent-FPU
ESC semantics and its existing timing owner; MyRaidenII's V30 profile is not
changed. This record does not admit new gameplay work or qualify T7.

Both existing Release Ninja graphs rebuild `myraidenii-0-0-0007`. The usual
release stripping is applied and version 0.0.0007 is retained. PE machine
fields are 8664 (x64) and 014C (x86). All 13 registered MyRaidenII unit cases
pass on each width within the repository unit results. The final incremental
builds report no work. Source, tests, configuration, ROMs, media and snapshots
remain unchanged; no gameplay, desktop/RDP or real-time validation is claimed.

| Artifact | SHA-256 |
| --- | --- |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `D7CCAD7858B1F59B10C1C9B6D1EC5B76C4F52714E730A04A7BF9753941A4CA4E` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `AD23D6F897966A30337A58C7756CAF70901DB9C07CA360F0FA302243ACBF1DC1` |

The NXVM S54 evidence owns complete cross-target regression and artifact
details. This record and pair form a separate MyRaidenII-target P; PC-family
source, tests and artifacts are not mixed into it.
