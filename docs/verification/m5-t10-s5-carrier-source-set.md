# M5 T10 S5 Carrier Source-Set Audit

The generated Ninja graph for `nxvm-m5-t10.exe` contains no implementation
from `src/nxvm-baseline/device/{device,vmachine,vcpu,vcpuins,vram,vport}.c`.
Those files no longer exist there. The artifact compiles the corresponding
implementations from `src/machine/vm/{device,vmachine}.c` and
`src/machine/core/{vcpu,vcpuins,vram,vport}.c`.

The remaining baseline C sources are explicitly deferred T11 controller,
firmware, platform, and utility providers. The legacy headers at the migrated
paths are forwarding compatibility headers only.

The S4 closure set passed: context lifecycle with the verified FDD fixture,
sequential CPU probe, FDD and HDD reset-vector smoke, no-media `help/info/exit`,
and `debug/q/exit`. The verified local task artifact is
`build/output/nxvm-m5_t10.exe` (ignored), SHA-256
`feae074aaa1f13134db27f991a41b4fd8c76fd1bcd84f8bb9724e4d8a444e08f`, built
from commit `b52b34c`. Its runtime banner is
`Neko's x86 Virtual Machine [0.4.015d.m5t10]`.
