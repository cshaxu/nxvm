# M5 T540 S36 Entry, ROM And Trace Boundary Audit

## Source and caller verdict

- `entry_plan_interface.c` reads only Core lifecycle/entry state, prepares the
  CPU entry, validates the requested memory route and non-overlapping RAM
  preloads, then commits through the existing CPU/memory route. Its production
  entry is a Core capability; present direct consumers are repository-only
  tests. It does not choose a profile, BIOS, port or board device.
- `rom_mapping_interface.c` owns one immutable mapping table. A primary image
  is copied before route publication; aliases point into a prior mapping,
  carry `owns_image = false`, and use overlay or reset-overlay route priority.
  Failed registration clears the candidate; firmware failure and destruction
  use the same reverse-order rollback. Its firmware-operation flags guard a
  Core configuration transaction, not an IBM-PC controller. The one concrete
  unused board-header dependency, `device_support.h`, was removed here.
- `trace_interface.c` owns the one bounded trace buffer and copies Core tick,
  timeline and CPU-PC values. Board callers in `board_advance.c` submit typed
  PIT/PIC/FDC/HDC/RTC/KBC/VADP effects; the trace implementation does not
  inspect those devices or hold their state. Core bus, memory, port and
  scheduler callers use the same record route.

These three implementations still include the present mixed `machine.h` to
reach Core fields. That is a compile-time boundary awaiting the S37 finite
file ledger and S38 physical move; it is **not** evidence that `x86/core` has
already been made independent. No second entry, ROM or trace route was added.
The production diff is **one removed include line**, with no ABI, algorithm,
timing, firmware-byte or asset-path change.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
- Current specialized gate target passed, including the direct strict-
  compilation matrix.
- External boot checkpoint passed once for Default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `46D03C88CB3D6CEE846D36AB951B2F3CD20688470715EDE5607C40AF0A6BBE5B`, `A4BAC18AAE56A498B9C80A5FE9994D70631C0B4E131623BB87827B42822765D4`.
  - Default: `564F7AFD2D9E46273FD68DEC4E4F86CE9F9EC52206232B99B00D408FE22E09AB`, `045DFE02ED1C94945FD01F594D6BCFFCB4F7EC4A24D59C02F327CB902DF157A9`.
  - XT: `F155394A75D5C3C20E29D2DFD99FD1819D9F2B3464AD9062CCCCBBDCB91BF599`, `ACCF0F9402DDC5E31F10DE86DBC7ED5923840E8761B310C266E3FA100F168528`.
  - 5170: `336499269C9F58854AF53BF7CC4BC0871FE4E12A02941376385DAB059C9CB637`, `FC68998243DA0AFD8DF57A0BCC92DE520434097B496C7DE46C2FFB36B06C11D4`.

No Shared/MyNES, owner INI, protected firmware, media or timing grade was
modified. T540 remains open for the complete S37 audit and later moves.
