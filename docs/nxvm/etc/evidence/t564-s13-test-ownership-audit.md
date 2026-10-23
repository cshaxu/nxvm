# T564 S13 — X86 NXVM negative verification and App test ownership

## Scope

This review covers the X86 NXVM corpus negative verifier and every direct
`x86-nxvm/machine/machine_private.h` include beneath the four PC App test
packages.  It does not alter production interfaces.

## Verifier

`test/x86-nxvm/verify_negative.cmake` previously copied the complete source
corpus once per CMake-edge and source-edge probe.  It now copies the corpus
once for each independent probe family, restores the altered input before
each case, and keeps independent build-bypass probes.  The latter matters:
each nested CMake statement now starts from the original top-level file, so a
previous illegal include cannot satisfy a later assertion by failing first.

The retained negative cases cover direct forbidden links, source edges,
nested test ownership, nested build descriptions (mixed case command,
directory link property, target property, and direct link command), and raw
CRT vocabulary.

Focused verification after the change:

| Build | Test | Result | Real time |
| --- | --- | --- | --- |
| x64 | `x86-nxvm.verifier-negative` | passed | 13.53 s |
| x86 | `x86-nxvm.verifier-negative` | passed | 27.93 s |

The earlier x64 baseline was 26.56 s.  The single later x86 result is slower
than its earlier 23.65 s baseline, so it is recorded as host variance rather
than claimed as a performance gain.  The structural result is nevertheless
smaller: two complete corpus copies rather than one copy per assertion.

## Private-header ownership inventory

The public X86 NXVM Machine interface intentionally exposes construction,
binding, media and fixed-machine operations, but not raw Core/Board pointers.
Publishing a raw getter solely to make these tests stop including the private
header would break the component boundary.  Every included test was reviewed
against that interface and against the generic X86 NXVM test package.

| App test package | Unit | Integration | Diagnostic | Support | Disposition |
| --- | ---: | ---: | ---: | ---: | --- |
| My5160 | 1 | 0 | 0 | 0 | retain: fixed 5160 ROM and composition |
| My5170 | 4 | 0 | 0 | 0 | retain: fixed 5170 clock, firmware/FDC and video topology |
| MyDeskPro386 | 20 | 0 | 3 | 2 | retain: Model 40 D4, CECG, ROM, media and board facts |
| NXVM | 29 | 17 | 4 | 1 | retain: default profile ROM, CMOS, EGA/CGA, guest media and diagnostic facts |

The 81 references are all App-selected-machine assertions or explicit App
diagnostics.  Generic component behavior is already owned by
`test/x86-nxvm`; none of the 81 is a generic component test misfiled under an
App.  No test moved, no test-only public API was added, and no assertions were
weakened.

## S13 boundary

This is a test-corpus maintenance change only.  It does not change guest
execution, timing qualification, App firmware, emulator behavior, public
headers, or generated executables.
