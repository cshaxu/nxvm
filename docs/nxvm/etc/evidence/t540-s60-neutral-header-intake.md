# M5 T540 S60 Neutral Core Header Intake

## Measured boundary

The planned one-step private-header closure exceeds a bounded S. Current
`src/app-nxvm/devices/machine.h` is **384 lines** with **133** direct source/test
includers. The public `machine_interface.h` is **694 lines** with **174** direct
includers. The private header still directly includes PIC, PIT, DMA, RTC,
FDC, HDC, KBC, XT PPI, XT keyboard, VADP and D4 interfaces despite the chip
instances already being board-owned. D4 memory has about **49** source/test
references; frozen-plan and topology families have roughly **54** and **81**.
These are different ownership and compile boundaries, not one include edit.

The neutral Core is not ready for a physical Shared move. S60 changes no
code and makes no false independent-build claim. The receiving allocation is
strictly numeric and linear:

1. **S61:** D4 memory value and board memory adapter ownership, preserving
   parity/mapping semantics and one Core checked-memory operation path.
2. **S62:** Separate the frozen plan's board topology/types from neutral
   declarations without copying the plan or adding a second publication.
3. **S63:** Remove remaining board-only private definitions/includes from
   the neutral Core header; migrate direct consumers by actual need.
4. **S64:** Split the public interface and prove an independently compiled
   neutral Core, without PC topology or product includes.
5. **S65:** Physically move only the proven neutral source/tests to
   `src/x86/core` and `test/x86/core`, reconnect NXVM and delete old copies.
6. **S66 onward:** Extract individually audited IBM-PC common/AT/XT board
   receivers. Their end number is assigned from source evidence.

If a receiver still exceeds one safe owner boundary, split it into further
**numeric** S tasks. No letter suffix or parallel implementation path. The
old prospective "S61 physical move" was never executed; this supersedes it.

## Verification and artifact decision

S60 is a source/documentation intake only. Its own complete x64/x86 unit
runs pass **469/469** each. S59's both-width specialized gate sets and focused
firmware/scheduler tests **8/8** per width are the same executable baseline.
No source, test,
build, firmware, INI or EXE input changes here. The eight S58 booted
artifacts and recorded PE/hash values remain current. Documentation
governance and diff checks must pass before this packet closes.
