# T539 S78 Basic-Stack Receiver Evidence

## Result

S78 moves four CPU-only basic-stack test sources into the Shared CPU corpus:
GPR PUSH/POP, immediate PUSH, PUSHA/POPA, and ENTER/LEAVE.  Their existing
CPU-only fixture remains the sole implementation at
`test/x86/devices/cpu/support/cpu_instruction_fixture.h`.

The former NXVM registrations and source paths are deleted.  The public-board
stack tests remain NXVM-owned because they exercise actual Core stack memory,
fault delivery, PIC/IRQ timing, or machine wiring.  No production CPU source,
ABI, profile, firmware, INI, or asset changed.

## Verification

- Focused Shared receivers pass on x64 and x86.
- Complete repository-only unit suites pass 427/427 on x64 and 427/427 on
  x86 after the final registration graph.
- CPU/PIC authority, Shared test-manifest/corpus checks, documentation
  governance, and `git diff --check` pass.

The change affects tests and CMake registration only.  S76's committed 0539
product artifacts remain current; no executable rebuild was required.

## Commits

- `17359ef0f` — NXVM M5 T539 S78 P1, bounded the former oversized row.
- `b5dae6608` — Shared M5 T539 S78 P2, added the canonical stack corpus.
- `1833731fa` — NXVM M5 T539 S78 P3, retired the App paths.
