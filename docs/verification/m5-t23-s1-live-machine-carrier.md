# M5 T23 S1 Live-Machine Carrier

Root VM composition now binds `vm_composition_live_machine` directly to the
existing `vcpu`, `vcpuins`, `vram`, and `vport` objects. The carrier owns no
guest storage and does not alter retained initialization, reset, refresh,
Console, or debugger behavior.

Windows GCC built `nxvm-vm-live-machine-smoke`, `nxvm-m5-t23`, the retained
CPU-stop and full-PC profile smokes, and the dependency-DAG gate. The
live-machine smoke emitted `M5:T23:S1:LIVE-MACHINE:OK`; the retained CPU probe
emitted its expected reset-vector `#UD`; and the FDD/HDD full-PC smoke emitted
`M3:T3:S1:FULL-PC-PROFILE:OK`. A piped `HELP`/`EXIT` Console check retained its
original command surface.

The ignored local developer artifact is `build/output/nxvm-m5_t23.exe`.
Its runtime source is commit `43beb74`; the publication correction only adds
the CMake copy step. It emits `Neko's x86 Virtual Machine [0.4.015d.m5t23]`.
SHA-256: `33BF1BA6CC6D07C177952F8FD0FFC54CBBB6D9A02792C4AE8F4FD68C39E0FBB1`.
