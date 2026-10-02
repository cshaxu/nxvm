# M5 T540 S63 Private Core/Board Header Boundary

## Actual source cut

P1 `4d6443833` moves the frozen board plan, FDC/HDC topology and absent-
memory value definitions from neutral private `machine.h` into the existing
sole `machine_board_state.h`. PIC, PIT, DMA, D4, RTC, FDC, HDC, KBC, XT
keyboard and VADP concrete includes move with them. Board construction,
reset, wiring, deadline and plan-validation declarations now live beside
their board owner. `machine.h` retains only the opaque board attachment and
bounded callback value contracts needed by Core; no chip instance or board
plan definition remains there.

The complete build exposed five direct test consumers of the formerly
transitive board definitions plus one DMA fixture depending on the
transitive DMA chip include. Each now includes the actual owner. `machine.c`
includes the board header for its existing create/reset calls; this does not
claim independent Core compilation yet. No duplicate state, wrapper or
behavior route was added. The eight changed source/test files are **90
additions / 93 removals**, a net three-line reduction. Shared, MyNES, owner
INI and external input assets were untouched.

## Verification

- Complete repository-only units on final sources: x64 **469/469**, x86
  **469/469**. A concurrent x64 run saw one intermittent shared Win32
  `library.kvm_window_modal` failure; the isolated test and the final
  no-concurrent-build full run both passed. No shared source was changed.
- Both-width `verify-current-specialized-gates`: pass (82 targets each).
- One external boot per profile and width: Default `dos-prompt`; XT, IBM
  5170 and Model 40 `installer-running`; **8/8** pass.
- Eight optimized 0540 products: four PE x64, four PE x86, zero `.debug`
  sections. SHA-256 by profile, x64 then x86:
  - Model 40: `5652627B05D5A145D8EBDB0449A971D60890841B51DE5CB3C784DD8283F06EA9`, `8F5930298B2AB1C5C540647DB81E7AC7AF53BBB8007EA8A8CDDB02E33C2F8E02`.
  - Default: `29E960227047A8B05F754FE455C7A0366B720E1B9A00EE9ABF9C46240FF70D7D`, `BD300CAF9DD1C9199C45D83849DA69B14AD607A648A3856EE086F0BF85BB4DBC`.
  - XT: `A5D238CBB325278AD6F4B22F9317B01E85DC5B622F8651DEC5388EBC2D1B66C4`, `5FC7E08757AEDBBB27B46031A5825F0AA5CA1A3BCEDD979643DF7633F660F01E`.
  - IBM 5170: `40741DB9F10436DC02A00E7B5992BF95AB84803FD394FD4FE781A1FD597F36F2`, `DFF89954E269AF3BC869693A72DC37E09E9591F57CF5B83127EA6C269BB06167`.

The public `machine_interface.h` still contains PC-specific types and
includes; S64 owns that split and the independent neutral compile proof.
Physical Shared relocation remains S65. T540 stays open.
