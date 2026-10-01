# T539 S81 String Receiver Evidence

## Delivered Boundary

S81 makes the Shared CPU test corpus the sole owner of the CPU-only string
instruction suites: `MOVS`, `LODS`, `STOS`, `SCAS`, and `CMPS`.  The five
successor sources link only `x86-cpu`; their former NXVM copies and CMake
registrations are removed.  No runtime CPU algorithm, public CPU ABI, board
memory route, interruptibility path, PIC/IRQ path, profile, firmware, INI, or
asset changed.

Real Core memory, interruptibility, and PIC/IRQ observations remain owned by
their named NXVM board receivers.  The moved tests use the existing Shared
CPU fixture; S81 adds no second fixture, production path, or board adapter.

## Delivered Commits

- `39e71712f` — Shared P1 adds the five canonical Shared test receivers and
  updates the Shared test manifest.
- `b9919bcf1` — NXVM P2 retires the five App duplicates and switches static
  CMake/gate references to the Shared targets.

The tracked implementation paths add 1,821 and remove 1,844 lines (net
-23), excluding this evidence and state record.  The retained implementation
owner is the single existing Shared CPU execution path.

## Verification

- Focused Shared string receivers pass on x64 and x86.
- Complete repository-only unit suites pass **427/427** on x64 and x86.
- Shared manifest/corpus verification, CPU/PIC authority, documentation
  governance, and `git diff --check` pass.

This is test/CMake-only work.  It changes no executable input, so the existing
0539 product artifacts remain current and are not rebuilt.
