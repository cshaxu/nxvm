# M5 T85 Verification

- NXVM's outer loop calls `core_machine_run()` with a bounded one-instruction
  budget; it retains only host policy, command boundaries, display publish, and
  pause/reset/stop handling.
- The old `vmachineRefresh` and runtime `machine_refresh` callback are gone.
- `M5:T83:S3:CORE-LEGACY-RUN:OK` passed.
- `M5:T13:S8:VM-SESSION:OK` passed with `D:\fdd.img` and `D:\hdd.img`.
- `M5:T70:S2:DOS-PROMPT:OK` passed with `D:\fdd.img`.
- `M5:T45:S1:PAUSE-BOUNDARY:OK` and `M5:T14:S3:VM-DEBUG-TARGET:OK` passed.
- `verify-facade-ownership` passed.
- Task artifact: `build/output/nxvm_0_5_0085.exe`.
  SHA-256: `D5CD218DFD6F1BB0E1BF927C03B8F5DB768B845291E32AF78B006C3D01CF3F44`.
