# M5 T12 S8: Machine Directory Convergence Verification

The `machine/core/contract` subtree and the `src/core` forwarding tree are
absent. All source and test callers include `machine/core/*.h` directly.

The GCC build compiles the core contract, Machine lifecycle/trace/debug,
CPU-capability, PC-device, presentation, full-PC-profile smoke targets, and
the retained `nxvm-m5-t10.exe` target. Existing imported baseline warnings are
unchanged.

The following focused runtime checks pass:

- core contract, Machine instance/lifecycle/trace/debug, CPU-capability,
  PC-device, and presentation smoke executables;
- full-PC profile reset-vector smoke using the frozen local FDD/HDD fixtures;
- product Console smoke, presentation/debug smoke, and product-session smoke
  using the frozen local FDD/HDD fixtures.

The retained interactive Console's closed-stdin behavior remains historical
(it does not exit on EOF); its source and command path were not touched here.
The prior retained interactive Console/debugger evidence remains applicable.
