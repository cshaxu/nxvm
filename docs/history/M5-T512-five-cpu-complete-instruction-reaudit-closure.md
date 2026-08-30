# M5 T512: Five-CPU Complete Instruction Re-audit Closure

T512 closes the five-profile instruction, architectural-state and timing
re-audit without adding a profile-side CPU path.

## Accepted Result

The final 8086, 8088, 80186, 80286 and 80386DX result contracts cover 1,053,
1,053, 616, 771 and 1,413 instruction-timing keys respectively.  No final
CPU-instruction result is L1.  Manual ranges and coprocessor completion are
explicit External-L2 models at their one Core owner; exact manual values and
direct formulas remain Manual-L3.  The final cross-profile evidence is
[S9](../etc/evidence/t512-s9-five-cpu-final-tier-owner-audit.md).

The S8 same-class sweep corrected false 80386 `ESC`/`WAIT` L3 claims and
repaired the shared FPU false-completion defect.  Every valid CPU/FPU pairing
now reaches BUSY completion through Core elapsed-time advancement and a single
deadline, rather than a CPU-specific direct clear or VM/profile workaround.

## Verification

All five generated result contracts pass.  The repository-only unit suite
passes 315/315 and external-asset integration passes 20/20, including DOS,
FDC, HDD and Windows 3.1 checkpoint routes.  Documentation governance passes.
The stripped Release artifact is
`build/output/nxvm_0_5_0512.exe`, SHA-256
`45CA2B2167F64342B65371294BB2CF05DF11C167A79F666B925510FFE1049D14`.
