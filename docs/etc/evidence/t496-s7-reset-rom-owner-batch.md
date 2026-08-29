# M5 T496 S7 Reset-ROM Owner Batch

`M5:T496:S7:RESET-ROM-OWNER:IMPLEMENTED`

## Finding and retained design

Core already owns the sole guest-time axis and its device scheduler.  The
Default PC/AT boot failure was instead a reset-memory routing defect: an
ordinary high-address RAM mapping could satisfy a 286/386 reset fetch after
A20 routing had hidden the immutable ROM alias.  It therefore exposed zeroed
RAM rather than firmware bytes.

The retained route is singular for firmware-backed machines:

`firmware F0000h ROM -> Core-derived CPU reset alias -> reset-only raw ROM fetch -> ordinary low ROM after far transfer`

The raw route is restricted to instruction fetch/prefetch in the architected
286/386 reset window.  It does not alter normal A20 routing.  If no reset ROM
alias exists, Core retains its distinct fixture-only explicit RAM fallback;
that is not a firmware ROM path.

## Complete batch disposition

| Route | Disposition | Proof |
| --- | --- | --- |
| 80286 reset vector through derived ROM alias | accepted | New Core regression executes the reset far jump and reaches low ROM. |
| 80386 reset vector through derived ROM alias | accepted | Same regression proves the high reset-cache route and low-ROM transfer. |
| Firmware with only a short unrelated F0000h alias | accepted | Automatic alias admission requires the complete 15-byte reset prefetch source; existing firmware capability regression passes. |
| Normal A20-controlled memory access | retained | The reset-only operation is not used by data, page-table, or ordinary code fetches. |
| Firmware-less direct-Core fixtures | retained distinct route | Explicit backing-RAM fallback remains for configurations that do not register a reset ROM provider. |

## Verification

- Optimized Release focused Core/PC-AT/XT regression set: 5/5 passed.
- Optimized Release time, PIT, DMA/RTC, FDC, XT and Model 40 regression set:
  7/7 passed.
- Owner-authorized Default PC/AT 80386 BYOB replays reached: DOS prompt for
  1.44M and 1.2M media, date input for 720K media, and installer guest-input
  for 360K installation media.  No asset path, content, or hash is retained.

## Boundary

This batch does not make a BIOS containing 80386-only instructions compatible
with an 80286.  That is firmware/CPU compatibility work, not a ROM-routing or
FDC/timeline defect.
