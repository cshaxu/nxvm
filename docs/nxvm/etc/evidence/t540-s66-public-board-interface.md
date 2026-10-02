# M5 T540 S66 Public Board Interface

## Scope and receiving boundary

NXVM only. Shared six components, MyNES, owner INIs and external assets remain
unchanged. This is a declaration ownership cut, not physical Shared relocation.

The neutral `machine_interface.h` shrinks from 694 to 305 lines. The new
402-line `machine_board_interface.h` owns board constants, clock plan,
construction config, keyboard/XT enums, controller timing rules, display,
RTC/CMOS, parity/D4, absent-memory/alias and DMA topology, copied observations,
the opaque frozen plan, and their adjacent operation declarations.

All six migrated blocks match the original text verbatim. Structures, field
order, enum values, function names/signatures and function bodies are unchanged.
The board header includes the neutral header; the neutral header has no reverse
include or forwarding declarations. Generic CPU/DMA bus-ready operations stay
neutral. Private board construction-failure seams and clock-plan validation
declarations move from `machine.h` to the existing private board owner.

The public operations move with their values because a by-value XT fault enum
cannot be opaque-forwarded in portable C. Leaving its operation declaration in
the neutral header would restore the board dependency. There is no new wrapper,
second configuration, plan mirror, lifetime or runtime path.

## Caller and actual-diff review

The direct concrete-value inventory has 210 existing consumer paths: 26
production and 184 tests. Ignoring include lines, each of these paths is
identical to pre-S66 parent `9f8fb4802`. The explicit board-header receiver is also
included by private board state. Duplicate includes introduced during the
mechanical migration were removed before final verification.

Four profile implementation files previously obtained the same board values
transitively. Their direct owner includes are explicitly recorded by the exact
NXVM dependency-edge inventory, not a general layer exception. Display,
DMA/RTC and frozen-plan gates now inspect the actual declaration owner. The
controller gate additionally rejects board types and concrete board includes
in the neutral public contract and requires their board receiver.

Production: 30 paths, 440 additions / 410 removals, including the new header.
Tests: 184 paths, 185 additions / 104 removals; include changes only.
Static gates and exact dependency inventory: five paths, 30 additions /
5 removals. No behavior or timing classification changes.

## Remaining receiving work

S67's previously planned adjacent declaration cut is already completed
atomically here. Its measured implementation receiver is now the five neutral
validators still defined in board `machine_plan.c`: retirement contract,
timing capability, external-cycle timing, access-wait windows and transaction
contract. Their existing Core callers must not link board plan solely to obtain
validation. Move those bodies verbatim to an existing neutral owner, with one
definition each and no validator facade. S68 retains the four direct Core
reset/clock/NMI/finalization calls; S69 proves independent compilation; S70
owns physical Shared relocation. Independent Core build is not claimed here.

## Verification

Complete units pass **469/469** on x64 and **469/469** on x86, exit zero
(279.37 and 62.97 seconds while release builds ran). Both-width specialized
targets pass. Both public headers compile
individually under C11 `-Wall -Wextra -Wpedantic -Werror`. All migrated board
symbols are absent from the neutral header. Documentation and diff checks pass.

Default x64/x86 reach `dos-prompt`; XT and AT x64/x86 reach `installer-running`.
The preliminary XT x64 terminal handle was lost across continuation and no
terminal result was retrievable; it was not counted as a pass. The receiving
run records both XT terminal results and exit zero. Default was not repeated.
Model 40 x64/x86 also reach `installer-running`, exit zero. The receiving
matrix is **8/8**; each row has one counted terminal qualification. AT and
Model 40 are not repeated. These boot checkpoints do not replace the complete
T integration gate or prove indefinite absence of intermittent failures.

All eight optimized 0540 products are rebuilt in their existing profile
directories. PE formats are `pei-x86-64` / `pei-i386`, with no `.debug`
sections. MyNES binaries and owner INIs are unchanged. SHA-256 identities,
x64 then x86:

- Model 40: `2C2C5710780143409F7E18C977250B4F382A509F1F12087F7EC07562565D66C7`, `D880F3C240DF64B91907176C000A4DE180FA9FFBDE8BD9C6739EFCE54C77F194`.
- Default: `9C8DCB1DBCF516233BD351339FDE25435C2EAF6EA9D64EB18E6D3945939DE075`, `92C6A1FF3D258897608FC076A7539C33C8762B5461D60635168D4EAF99308BF0`.
- XT: `04866DECE8F5DF39189DB3B8E0B91FCDCA73668792F1A77C9AE722A4E55CF61D`, `7BFF4A29E86E8D30F51CC8D5B0152FF0BA52544FDE125E8A1E52FFDC04B6873B`.
- IBM 5170: `AD2ECDCAC1B948B719734A4ADFDE9F79935E6D2CF45D71D61E9CF6986EF36CBB`, `2B3D2F12D595953CA8872A72D2CD8A1B8BFDECCB9067900DDB47BF0E3A8F866D`.

## Actual P1 acceptance

Pushed NXVM P1 `455e920fc` has exactly 231 scoped paths and passes
`git show --check`. Coordinator actual-change review rechecks all 210 concrete
consumer paths against the parent: zero non-include differences. The value/API
cut, private seams, changed gate owners and four explicit existing profile
dependencies match the admitted boundary. HEAD equals `origin/master`, with a
clean worktree at review. S66 is accepted; S67 receives the measured neutral
validator implementations. T540 and the complete board extraction remain open.
