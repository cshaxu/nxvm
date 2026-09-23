# T429 S2: D4 and CECG Memory Classification

`M5:T429:S2:D4-CECG-MEMORY-CLASS:OK`

Tier: `generic-at`. This receiver does not assign a Compaq physical memory
cycle duration. It repairs the explicit applicability boundary for the existing
D4 logical 2 KiB page policy.

Core external-cycle timing now has an optional inclusive eligible-memory range.
The all-zero range preserves existing profile behavior. Model-40 explicitly
selects `00000h` through `9FFFFh` for D4 conventional-RAM page timing. The
VADP-owned CECG aperture at `A0000h` through `AFFFFh` is therefore not charged
as D4 RAM. CPU lifecycle, cancellation, reset and retirement remain in Core;
VM binds only the Model-40 range. 5170 retains its legacy all-memory selection.

`core-machine-prefetch-locality-smoke` proves a D4-range access charges the
existing miss and an A0000h access does not. Model-40 and 5170 composition
smokes prove selection and isolation. This does not model CECG aperture waits,
IOCHRDY/NOWS/BUSRDY, D4 PAL behavior, raster/monitor timing, or physical/L3.