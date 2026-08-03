# M5 T84 Verification

- `core_machine` owns the PIC, PIT, DMA, KBC, and VADP storage; the VM
  composition borrows those same objects.
- VM/profile-only CMOS, media, BIOS, and QDX reset/refresh through the frozen
  `core_machine_execution_provider`.
- `verify-facade-ownership` passed.
- `M5:T83:S3:CORE-LEGACY-RUN:OK` passed.
- `M5:T13:S8:VM-SESSION:OK` passed with `D:\fdd.img` and `D:\hdd.img`.
- `M5:T70:S2:DOS-PROMPT:OK` passed with `D:\fdd.img`.
- Task artifact: `build/output/nxvm_0_5_0084.exe`.
  SHA-256: `01A2085167DCAB67C927F074BEC035095B9F0CC27C968EA5E36AC9965DFE68E7`.
