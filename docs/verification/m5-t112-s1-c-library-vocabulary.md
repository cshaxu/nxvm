# M5 T112 S1: ISO C Vocabulary

## Result

Active project source and tests now use the `STD_*` facade for ISO C library
types, objects, constants, and atomic operations. `type.*` retains native ISO
spellings only to define or implement that facade. The atomic aliases directly
map to the C11 primitive and preserve each original explicit memory order.

## Verification

- Source scan outside `src/type.*` and `src/nxvm-baseline/`: no active native
  `size_t`, `ptrdiff_t`, `FILE`, `time_t`, `va_list`, standard stream/object,
  `SEEK_*`, or atomic vocabulary remains. The only `NULL` matches are one
  assembler display string and two comments.
- `cmake --preset mingw-gcc-x64` and
  `cmake --build --preset nxvm-current-gates-gcc`: passed.
- `nxvm-vm-dos-prompt-smoke D:\fdd.img`: emitted
  `M5:T70:S2:DOS-PROMPT:OK`.
- `nxvm_0_5_0112.exe` accepted `help`, `info`, and `exit`; retained Console
  output and idle device state were observed.
- Runtime identity: `Neko's x86 Virtual Machine [0.5.0112]`.
- Developer artifact: `build/output/nxvm_0_5_0112.exe` (not a release;
  contains no guest media), SHA-256:
  `1D951550B6D661053AF41A83FAA33D8235F7A93F97EB0A88B6E88203335F1015`.
