# M5 T21 S1 Trace Interface Rename

`src/core/machine/trace.c` and `.h` were renamed with `git mv` to
`trace_interface.c` and `.h`. Direct include and CMake paths now use the new
name. The trace event ABI, sink callback semantics, and public
`nxvm_core_trace_*` symbols remain unchanged.

GCC build, dependency-DAG, trace/core-contract, and retained Console
startup/exit gates passed. The local developer artifact is
`build/output/nxvm-m5_t21.exe`, SHA-256
`20187BBCAB06D6C44E25026C97BC2E904784ED21AD5C74E025825EE032F2CE95`.
It emits `Neko's x86 Virtual Machine [0.4.015d.m5t21]`.
