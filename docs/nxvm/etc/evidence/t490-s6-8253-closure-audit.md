# T490 S6 IBM 5160 8253 Closure Audit

`M5:T490:S6:8253-CLOSURE:ACCEPTED-PENDING-GOVERNANCE`

## Complete unit chain

- S1 visually verified the Intel 8253-5 and IBM 5160 sources and excluded the
  incorrect 8254/Read-Back superset.
- S2 froze all 22 selected function/timing rows; S3 mapped every row to one
  existing Core PIT/board/scheduler owner and admitted one finite batch.
- S4 added only the immutable two-value PIT personality at the existing
  `t_pit` owner. IBM 5160 selects 8253; `SC=11` is inert there, while default
  and auxiliary 8254 paths retain Read-Back. No XT PIT state, port provider,
  scheduler, profile runtime setter or VM branch exists.
- S5 corrected the pre-existing retirement-observation gate oracle: `F6 E0`
  and `9B` are unprefixed, while existing `F3 90` probes still prove REP
  presence. The CPU timing producer and retirement capture/publish route were
  reviewed; neither required a production change.

The final implementation delta is 37 `pit.[ch]` lines in S4 plus the S5
three-line smoke-oracle correction. It removes the unconditional 8254 label
and adds no duplicate state or execution path.

## Verification and artifact

- Focused CTest: 6/6 passed (retirement observation, 8253, 8254 Read-Back,
  waveform, IRQ0 and XT profile).
- Full configured CTest: 300/300 passed.
- Release configuration and build completed with the configured optimization
  and strip verification: `cmake --preset mingw-gcc-x64-release`; `cmake
  --build build/mingw-gcc-x64-release --target vm-0-5-0490`.
- `build/output/nxvm_0_5_0490.exe` SHA-256:
  `DB51B6B7564F683DCD30240FE1739518459DA82868F475829B3AAF6E7DFC24DA`.
- Documentation governance passed after the artifact identity cutover.

## Retained boundaries and transfer

XT PPI port-61 board bits remain the 8255 unit; physical-axis conversion is
L2, chip startup state L1 and electrical timing L4/out of scope. T490 has no
unrecorded PIT production gap and introduces no follow-up debt.
