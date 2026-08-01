# M5 T8 S1 Execution Migration Map Verification

The M5 artifact target was inspected with its CMake source set and runtime
entry chain. Its final executable currently compiles every Windows baseline
source, including `main.c`; `products/nxvm/pc_at.c` separately reaches the same
engine through `nxvm_baseline_full_pc_*`. The real guest executor is
`device/vcpuins.c`, reached from `deviceStart` and `vmachineRefresh`; the
existing `core/machine.c` does not execute instructions.

`docs/history/m5/planning/m5-execution-migration.md` records the destination ownership,
global-state breaks, regression gates, and T9 through T13 order. It preserves
the retained Console/debugger surface as an explicit stop condition. This is a
design result and produces no executable.
