# M5 T110 S1: ISO C Call-Site Migration

## Result

Active source and test callers now use the `STD_*` facade for allocation,
memory/string, file, time, ctype, and bounded-formatting operations. The
facade implementation is an explicit base CMake target, so platform-only
targets do not acquire a `core/machine` dependency.

## Verification

- Source baseline: `4b266ed` (`M5 T109 S1`).
- Targeted direct-call scan outside `src/type.*`: clean.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0110.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0110]`.
- Developer artifact: `build/output/nxvm_0_5_0110.exe` (not a release;
  contains no guest media), SHA-256:
  `2D2608576CA7A6B31C6C5F72BCC8EF66B8F6C03C22B4FB6675FAF9D42614D45A`.
