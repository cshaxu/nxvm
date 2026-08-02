# M5 T20 S1 Keyboard Interface Rename

`src/core/machine/keyboard_state.c` and `.h` were renamed with `git mv` to
`keyboard_interface.c` and `.h`. Direct includes and the core-machine CMake
source path now use the new name. Public provider symbols and all runtime
behavior remain unchanged.

GCC build, dependency-DAG, core-contract, and retained Console startup/exit
gates passed. The local developer artifact is
`build/output/nxvm-m5_t20.exe`, SHA-256
`986F6804602B4A590C8AA6D3E353360D5178CA4FB42CD6B42EA317873873EDB2`.
It emits `Neko's x86 Virtual Machine [0.4.015d.m5t20]`.
