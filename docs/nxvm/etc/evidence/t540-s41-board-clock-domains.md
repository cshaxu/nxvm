# M5 T540 S41 Board Device Clock Domains

## Actual source and owner diff

Six existing named board-device domains—DMA, main PIT, auxiliary PIT, RTC,
VADP and KBC—now initialize and reset together in the existing board
advance owner. Their order, ratio inputs, phase reset and failure status
are unchanged. The neutral construction path continues to initialize only
the provider clock; the Core cold-reset path still resets its one guest
timeline and then invokes board-clock reset at the original position before
resetting the provider clock. The frozen plan is read once; no clock state
is mirrored and no second guest-time axis was introduced.

The board clock initialization occurs before board port registration and
publication. A failed initialization still returns `INVALID_ARGUMENT` and
destroys the candidate; the common preflight validates all seven ratios,
so a valid frozen plan cannot newly fail after Core allocation. FDC/HDC
have no independent named clock domains in this implementation; their
deadline/service contracts remain unchanged. S42 still owns the mixed
private-state and constructor split.

Three NXVM production files changed: **39 lines added, 19 removed, net +20**.
The T388 physical-timebase static inventory gained `board_advance.c` as an
input without dropping a required token. Shared/MyNES source and tests,
owner INIs, firmware inputs and timing formulas are untouched.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
  The first x86 run had two load-induced 30-second timeouts; each passed
  in isolation, then the entire suite passed at bounded concurrency.
- NXVM `verify-current-specialized-gates` passes, including the updated
  T388 source-owner inventory and direct strict-compilation checks.
- External DOS boot checkpoint passed once per fixed profile and width:
  Default, XT, IBM 5170 and Model 40, **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` confirms four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `F15F850EDE93EC3A11245753EE61C91AACDBB573033F08CF8AFEB5A0EF5CC8E1`, `39C30AD42605524BC494EBE424E9AC3A8C2427CF04A192C0EC28C135A203DF65`.
  - Default: `40195D2BB60822D309E9B6C207A4F3F34B0CE113E6B8512382EE1D7D5B2B8A9A`, `DFB4E45C5BC564AC669084446C35D25A462168FC07359C4E3130B50EE7128C89`.
  - XT: `8B4CA5FE9183E7B0BE310211E5892DB107C760E5286D384CDBEF6E5F1C833407`, `7393F01D2428A9576358F360204BDEC455820C74BD5A47D1DB85EED6B6B0A33B`.
  - IBM 5170: `FA6A4D469AABDAF869E70BF7B28DA44C98DE7D3A414270B5E4ABE083500F739B`, `6E837053A9D6BF49DBD06B8842BD292D7783DBE9157BCB173234E2A33B3C72AA`.

T540 remains open for S42-S43 and later IBM-PC board extraction.
