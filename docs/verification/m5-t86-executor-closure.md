# M5 T86 Verification

- `verify-executor-closure` emitted `M5:T86:EXECUTOR-CLOSURE:OK`.
- The CPU probe now executes through `core_machine_run()` and emitted
  `M5:T1:S1:CPU-PROBES:OK` (the deliberate #UD probe diagnostic is expected).
- `M5:T70:S2:DOS-PROMPT:OK` and `M5:T45:S1:PAUSE-BOUNDARY:OK` passed.
- Obsolete tests that asserted VM-owned CPU/RAM/port/PIC/PIT/DMA/KBC/VADP
  storage were removed; `nxvm-vm-core-executor-storage-smoke` is the retained
  ownership coverage.
- Task artifact: `build/output/nxvm_0_5_0086.exe`.
  SHA-256: `C7743DFCD264116B0B4FC4484D9CE143C76AAA661AB5873CCC82ECB4BEA6C882`.
- An all-target GCC build compiled the remaining targets but could not finish
  linking historical artifacts `nxvm-0-5-0050.exe` through `0053.exe` because
  Windows reported those output files locked. Re-run after their handles close.
