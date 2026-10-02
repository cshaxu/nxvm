# M5 T540 S34 Cold-Reset Boundary

## Actual ownership diff

The sole `core_machine_cold_reset` still performs CPU/FPU, port and memory
reset before the board phase, then resets Core stop/fault, counters, time,
transaction and provider state, invokes firmware, and publishes the original
STOPPED result. On firmware failure it still leaves the machine INITIALIZED.
Its existing `core_machine_processor_reset` remains CPU-local: it does not
reset RAM, board devices or scheduled time.

One private `core_machine_board_reset_devices` now owns the contiguous board
phase. It preserves the exact previous order: D4 memory; exclusive XT
PPI/keyboard or AT KBC plus input pins; DMA; RTC; parity/D4 board latches;
FDC; HDC; PIC; system and optional auxiliary PIT; post-PIT board wiring;
D4 refresh state; video. The old small board-state reset wrapper was inlined
into this sole phase, not retained as a second route. No device reset
algorithm, clock formula, state owner or public API changed.

The production diff changes three NXVM files: **30 lines added, 32 removed,
net -2**. Teardown remains S35 work; ROM/entry/trace qualification remains
S36. Shared and MyNES were not modified.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
- Current specialized gate target passed, including the direct strict-
  compilation matrix.
- External boot checkpoint passed once for Default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `87D545B4DAC509F468EAC68F9B691828E49FDC3970C4D64CAF27814F93D2F4FF`, `893A2D12A4D377E94D5E223B08AEEA92F4F62AF6FFDA64C95DC697F1D97A7054`.
  - Default: `EE779888B6209E05A6B5349F57BB63B21D958E4CE33FBCB54DDD124B831B4CF4`, `5768F2F32EDB880CD3D10D1F1B863A0379732AAEA9B9282D4F0B4F1A0FD3B1D1`.
  - XT: `E6078A5C8AA291CF409E8025A4BFB13DBF9BB1B4F6734023215BED6AEA28DC04`, `C23D86232C7614E24503AF47021322D9905FB41AF907ABE11A99267887332C82`.
  - 5170: `B0F84BBF070FA5A60F86A841BFD9490AE8220D717FBE263F7792DE116FCE3FC5`, `D81ADE900D6EE6AC8AB853F522C5DFA5C7F16AAF6963BDAF45073C82E9DA6A17`.

No owner INI, firmware input, protected asset or timing grade changed. T540
remains open for teardown and the later physical extraction.
