# M4 T26 MyNes Identity Rename Evidence

## Scope

T26 applied MyNes to product-facing strings and mynes to technical identifiers.
It covered tracked source, CMake targets and presets, test targets and fixture
names, retained executable names, governance tooling, documentation and history.
The local checkout path is not a product identifier. iNES, NES, SoftPC and NXVM
remain distinct protocol or external-project names.

## Verification Record

| Check | Result |
| --- | --- |
| Tracked text/path identity sweep | Passed; no retired product identifier found. |
| Documentation governance | Passed for mynes-0-1-0011. |
| Documentation governance self-test | Passed. |
| x64 configure/build and CTest | Passed; 110 registered tests. |
| x86 configure/build and CTest | Passed after regenerating a stale build tree; 110 registered tests, with the repaired type-layout self-test separately re-run and passed. |
| Product prompt smoke test | Passed; command smoke verifies MyNes prompt. |
| Retained artifacts | mynes_0_1_0011_x64.exe and mynes_0_1_0011_x86.exe exist beside mynes.ini. |
| Owner ROM policy | Passed; assets/roms has no tracked files and is ignored by the root assets rule. |

## Artifact Hashes

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_1_0011_x64.exe | EBB638A1BEA7DD6E0002CDFF5525B589A43D90BEBCEA37A04AC19F38F462471B |
| mynes_0_1_0011_x86.exe | E9AB2EA59F3C7023EFA2AE625215B8253D5A068D1ECFCC63D4685BD4AA7FB411 |

## Delivery

No remote exists for this repository. T26 therefore closes through its local
commit series under the approved remote-aware rule.
