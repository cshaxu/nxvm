# M5 T126 S1: No-op Facade Removal

`vm_platform_initialize/finalize` and `vm_machine_hdc_*` lifecycle functions
had no state or behavior and are removed. `VHDC_INT_SOFT_HDD_13` remains as
default-profile firmware, its real owner. The process-static Win32 Console and
Linux terminal leases remain: each is an atomic, explicit, process-exclusive
claim over one physical host surface, not session state. T127 must verify that
they have no hidden session, renderer, or input state and that every future
exception is recorded in the mutable-state inventory.

Verification: `cmake --build --preset nxvm-current-gates-gcc` passed. The
task artifact is `build/output/nxvm_0_5_0126.exe`
(`502E820B5535FD1D06D00147FE1E0B0B0E1F9301F62AD04AB9F69347F6BD98C9`).
