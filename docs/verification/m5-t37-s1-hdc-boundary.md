# M5 T37 S1 HDD-Controller Boundary

The retained `vhdc` implementation has no data object, port registration,
DMA/IRQ callback, BIOS service link, or media authority. Its `Init`, `Reset`,
`Refresh`, and `Final` hooks are intentionally empty. T37 therefore adds no
invented state; it preserves this boundary for later controller implementation.

Windows GCC, `M5:T37:S1:HDC-BOUNDARY:OK`, FDD/HDD full-PC profile,
debug-target, retained Console, and dependency-DAG gates passed.

Artifact: `build/output/nxvm-m5_t37.exe`.
SHA-256: `E4E18A2A1CB2717867D4CC86625DB797005AD79B313760F7ADE35F8B73C7310B`.
