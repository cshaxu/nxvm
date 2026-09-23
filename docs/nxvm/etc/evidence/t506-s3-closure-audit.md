# T506 S3: DMA Closure Audit

## D1--D10 reconciliation

T506 S1's selected DMA ledger is closed as one owner batch.  D1--D5 retain
their Manual-L3 request, priority, arbitration, phase, mode and completion
relations in Core DMA.  D6--D8 retain the selected PIT1 refresh, FDC DMA2 and
XT Xebec DMA3 routes; their producing controller deadlines remain separately
owned.  D9 retains reset and withdrawal through the existing binding lifecycle.
D10 is closed for the existing qualified Model-339 input: the scheduler uses
the copied `3/8` DMA clock to reach the next sourced service phase.  All other
profiles retain their existing lower-tier declarations; neither a fixed service
duration nor a provider timer was introduced.

## Verification

- Focused `unit.core-machine-plan-smoke`: pass.
- Complete repository-only unit suite: 312/312 pass, `ctest -L unit -j 8`.
- Complete owner-managed external integration suite: 20/20 pass,
  `ctest -L integration -j 8`.
- Release configuration and `current-gcc` build: pass.
- `build/output/nxvm_0_5_0506.exe`: banner `0.5.0506`, optimized Release,
  runtime debugger retained, no `.debug`/`debug_` PE section, SHA-256
  `E93BD212891464F03E8FD912589C5F2490D9F6B4E5EBCE3EF1C537319E3DB116`.

No external asset path, hash, bytes, trace or third-party source entered the
repository.  The only tracked executable-path delta in T506 S2 is `+68/-1`
source/test lines; S3 changes only the current artifact identity and closure
records.  The retained production path is one Core scheduler, one clock domain
and one DMA service state owner.
