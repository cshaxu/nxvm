# M5 T9 S2 Console Source Move Verification

## Source Move

The original NXVM files were moved, preserving their copyright notices:

- `main.c`, `console.c`, `console.h`, `debug.c`, and `debug.h` to
  `src/product/vm/`.
- `xasm32/aasm32.c`, `aasm32.h`, `dasm32.c`, and `dasm32.h` to
  `src/product/core/debug/xasm32/`.

The only source-path adaptations are the two include paths in
`src/nxvm-baseline/utils.c`, the two moved assembler source includes, and the
two moved assembler header includes. `utils.c` and `utils.h` remain in the
baseline tree; its device and platform consumers were not changed.

## Build Evidence

`cmake --build build/mingw-gcc-x64 --target nxvm-m5-t9` completed successfully
with the project MinGW-w64 GCC toolchain. The target source set names the moved
Console/debugger and assembler/disassembler paths directly. Existing NXVM GCC
warnings remain and are not modified by this task.

The ignored task artifact is `build/output/nxvm-m5_t9.exe` with SHA-256
`FDD7163B8EF1A527A71F54D4E1694C20C93B31DE1FEAE4F4057E59914240CA10`.

## Behavior Evidence

With no guest media mounted, the executable reported
`Neko's x86 Virtual Machine [0.4.015d.m5t9]` and passed:

- piped `help`, `info`, `exit`, including the original `HELP    Show help info`
  and `NXVM Device Info` markers;
- delayed interactive `debug`, `q`, `exit`, retaining the original
  `Console> -` debugger entry and return to `Console>`.

No Console command grammar, text, debugger behavior, default workflow, or
window/input behavior was intentionally changed.
