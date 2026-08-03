# M5 T122 S1: VM Test Models

CPU probing and the compact alternate Console/debugger models remain covered by
their smoke tests, but their implementation now lives below `tests/` and is
not compiled into `nxvm.exe`, `nxvm-vm-composition`, or `nxvm-vm-product`.
The retained interactive Console and composition debug target remain the only
production product paths.

MinGW-w64 GCC 16.1.0 gates passed: CPU probe, compact Console model,
presentation/debug boundary, two-session isolation, and retained Console
lifecycle. Developer artifact: `build/output/nxvm_0_5_0122.exe`, SHA-256
`38D9B615973A76CFD50FEBFC9DA10DF402F84CA632D883F031978E8647B75E64`.
