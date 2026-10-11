# X86

X86 owns shared x86 debugger/assembly tools and the `x86/product` command,
hotkey and composition policy. It depends only on Lib and Emulator.
`emulator/product` owns neutral Machine/Session/UI construction and ordered
teardown; X86 owns the PC command implementation and surface binding used by
both NXVM and SoftPC.
