# Product

Product owns shared PC user-facing command policy, debugger/assembly tools and
surface composition. It depends on Lib and Emulator. `emulator/product` owns
neutral Machine/Session/UI construction and ordered teardown; this component
retains the PC command implementation, hotkey policy and surface binding used
by both NXVM and SoftPC.
