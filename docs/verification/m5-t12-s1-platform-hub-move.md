# M5 T12 S1 Platform Hub Move Verification

The original NXVM platform hub now compiles from
`src/vm/platform/platform.c`; its header is `src/vm/platform/platform.h`. The
retained baseline header forwards remaining callers to the new owner. Direct
machine, Console, and adapter callers include the new header, while Win32 and
Linux leaf providers remain in their existing locations.

The legacy `utils <-> platform` static-link cycle is retained inside the
temporary baseline-runtime archive so the original resolver behavior is
preserved; source ownership remains `platform/vm`. GCC built the user-facing
target and focused smokes. The FDD-backed execution lifecycle, sequential CPU
probe, FDD/HDD reset-vector smoke, no-media Console, and debugger-prompt
regressions passed. No Console/window selection, platform lifecycle, input, or
display behavior changed. The canonical-root follow-up also built the
user-facing target and finite CPU probe, retaining `M5:T1:S1:CPU-PROBES:OK`.
