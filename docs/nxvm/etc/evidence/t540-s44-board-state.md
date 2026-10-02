# M5 T540 S44 Board-Owned State

## Actual diff and owner review

The six device clock domains, KBC timing/input values, keyboard/display
topology, DMA bindings, RTC/CMOS configuration, and FDC/HDC topology moved
from 27 flat `core_machine` fields to one `core_machine_board_state`. The
Core machine has one opaque board pointer. The existing board constructor
allocates the state before clock and device initialization; its existing
failure path destroys the machine, and the board finalizer releases the
state after chip finalization. Neutral construction can still fail before
board allocation, so finalization accepts a null board pointer. No clock
phase or topology fact is mirrored.

Board advance/deadline, construction, display and plan readers now use the
one attachment; direct white-box fixtures follow the same path. The VM
machine's distinct `fdc_dma_request` binding remains VM-owned and was not
moved. Core's `provider_clock`, `time_axis` and timeline remain unchanged.
The two board-source inventories were updated to require the new clock
field paths while forbidding them in the neutral scheduler. Tracked source,
gate and test changes add 217 and remove 214 lines; the new private header
adds 38 lines (net +41). The churn is field access and include migration,
not another implementation. No public API, chip algorithm, timing formula,
profile, INI, firmware or media input changed. Shared and MyNES are untouched.

## Final-source verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including board
  inventories, strict compilation, manifests and documentation governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt` on x64/x86; XT, IBM 5170 and Model 40 reach
  `installer-running` on x64/x86. **8/8** accepted terminals.
- Eight optimized 0540 EXEs were rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. `objdump -f` reports four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds zero `.debug`
  sections. SHA-256 by profile, x64 then x86:
  - Model 40: `79DC099932C018DBE8F8300C5488C96B6558B45C6E5C61F266956B1B3CD6671A`, `BAD1D63779D2AEBC085D1AA47DB18A5918BDBB7BF691ED21769F391D89470BED`.
  - Default: `C95A05FAD4515C33451D1CA8EC25B2A9F18F28E1BFC7A3A7BBA0E203B04A155C`, `08D7F57E2D7FE42A75262ADE3729FE9E6EAEE81F540CB1603FB8D94E7C7E89AE`.
  - XT: `E6396009684C2E17705746EB35CBD0588CF645AD911CB2E638B7856A52E23CCA`, `1CA568A92A47E3C951EDD55BF4CFDA7F27B9BC03089D44DFDE88CCF6633F9D76`.
  - IBM 5170: `FBDECBF9AAF390F50EEF3C0CA92919D0434B54ABCA5A257B6BB9750849F0B961`, `0CD00A45F1A70B370D6469DC37C4824D7680A613F5196AF42767007D773A6744`.

S44 closes only named board clock/topology state ownership. Chip instances,
board latches/callbacks, neutral private header separation and physical
Shared relocation remain in S45-S48. T540 stays open.
