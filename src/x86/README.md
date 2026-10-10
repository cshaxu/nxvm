# x86

This portable x86 corpus owns shared PC command policy, debugger/assembly tools
and product composition. It depends on Lib and Emulator. `emulator/product` owns
neutral Machine/Session/UI construction and ordered teardown; this component
retains the PC command implementation, hotkey policy and product binding used
by both NXVM and SoftPC.
