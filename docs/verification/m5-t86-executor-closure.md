# M5 T86 Verification

- Default GCC build passed after historical task-artifact targets were excluded
  from `ALL`; each remains individually buildable.
- `verify-executor-closure` emitted `M5:T86:EXECUTOR-CLOSURE:OK`.
- The CPU probe now executes through `core_machine_run()` and emitted
  `M5:T1:S1:CPU-PROBES:OK` (the deliberate #UD probe diagnostic is expected).
- `M5:T73:S1:TWO-SESSION-ISOLATION:OK`, `M5:T13:S8:VM-SESSION:OK`,
  `M5:T70:S2:DOS-PROMPT:OK`, `M5:T45:S1:PAUSE-BOUNDARY:OK`, and
  `M5:T14:S3:VM-DEBUG-TARGET:OK` passed.
- Obsolete tests that asserted VM-owned CPU/RAM/port/PIC/PIT/DMA/KBC/VADP
  storage were removed; `nxvm-vm-core-executor-storage-smoke` is the retained
  ownership coverage.
- Task artifact: `build/output/nxvm_0_5_0086.exe`.
  SHA-256: `1E6DE3DE339815BA8DB6412DA8DA46B2627FB993464D5CF6D2E1A330B1CE0AF9`.
