# X86 IBM PC Product

Standalone CMake projects are `shared-x86-ibmpc` (source) and `shared-test-x86-ibmpc` (tests).

Owns PC command/debug binding, keyboard hotkey policy and product entry.
Uses Lib, Emulator and x86-debug. Emulator Product remains the sole neutral
composition/session lifetime owner. App supplies a composed machine and optional
command/snapshot callbacks. No machine information or speed policy is defined here.
