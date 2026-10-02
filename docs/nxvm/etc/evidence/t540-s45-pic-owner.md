# M5 T540 S45 Board-Owned PIC Pair

## Scope and actual diff

The S45 pre-edit inventory found that moving every remaining chip in one S
would cross independent lifetimes. The [receiving ledger](../architecture/t540-s42-private-state-ledger.md)
now assigns PIC, PIT, DMA, RTC, FDC, HDC, keyboard and VADP to the linear
S45–S52 sequence; S53–S55 retain electrical state, neutral Core isolation
and physical relocation. S45 therefore moves only the PIC group.

The PIC master/slave instances and PIT0/RTC PIC IRQ-source bindings are now
stored in the existing sole `core_machine_board_state`. They were removed
from flat `core_machine`, with no mirror or accessor layer. The same board
constructor binds the same PIC topology and sources in the same order;
reset, interrupt scan/acknowledge, deadline, port transaction and finalizer
call the same chip operations through their new address. Board allocation
and release remain the S44 path. Core CPU bus still sees only the bounded
PIC pending/acknowledge callbacks, not PIC state.

PIC field consumers appeared in three production files and fifty direct
test files. Their field access and required private include were migrated;
three exact source-inventory gates now recognize the board path and continue
to forbid concrete PIC effects in Core/CPU. Tracked source/gate/test diff
adds **363** and removes **314** lines (net +49); the size is chiefly direct
fixture references, not a new implementation. No public API, IRQ algorithm,
chip model, timing formula, profile, INI, firmware or media input changed.
Shared and MyNES files were not edited.

## Final-source verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including the PIC
  locality/CPU authority checks, strict compilation, manifests and docs.
- Fixed-profile external boots, **once per profile and width**: Default
  reaches `dos-prompt` on x64/x86; XT, IBM 5170 and Model 40 reach
  `installer-running` on x64/x86. **8/8** accepted terminals.
- Eight optimized 0540 EXEs were rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. Build artifact verification reports
  four x64 and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `0430EF4937F224D658EEC4FD928CBFAB8E6269E92F3CFED86B2B029A1312135F`, `150E522730293845FEC3853926E5E4294900E9E9CC7DAA2AC0DA664996EE63D7`.
  - Default: `F4F9FB046DCAF5315598F763539E31B0DF757CFA47CBB1268D138A6A947D29AE`, `BD4942B32C1445A5AD43A27EBFBAA661CA84E665AA3EE89C365E80B4EE2B6308`.
  - XT: `CCF0CC49CA7FDA05F6499F01D79A50778C549D091EEEDF9C18D4A9A2445D801A`, `C940BAD5FCC34CD554D8C267C88DADE46205206DDB1E8E119FE78660BEB1638F`.
  - IBM 5170: `468FCA57A0ECEECEF30678EB2224F3749C8B0914EBB3C4094920926F04999F7A`, `53E5F26FB208B39DAEBD2E672FBD63553DD5F2389EA39A8CD35214A1A237FBE2`.

S45 closes only the PIC group. S46 receives PIT instances; T540 stays open.
