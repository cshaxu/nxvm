# T498 S5 80386 MOV Segment-Register Regression

`M5:T498:S5:I386-MOV-SREG-REGRESSION:OK`

## Authority And Scope

The source authority is Intel 80386 DX PRM (1990), Chapter 17, printed page
17-114.  This S consumes only the complete `8C /r` and `8E /r` segment-register
MOVE direction/form batch required by the current packet; it does not change
the generic scheduler or any profile boot path.

## Verification

- `tools/Verify-80386DecoderLedger.ps1 -RepositoryRoot .` reports the manual
  decoder partition and zero-difference canonical ledger.
- The directly compiled Release
  `core-machine-80386-timing-manifest-runner` reports all 1,413 canonical
  records closed, including every non-control, string/I/O, ordinary-control
  and protected-control coverage/input marker.
- The retained result manifest records both `I386-MOV-SREG-LOAD` (`8E`) and
  `I386-MOV-SREG-STORE` (`8C`) as Manual-L3, `passed: true`, and
  `source_timing_unallocated: false` in base, 16-bit, 32-bit and legal
  segment-override contexts.

## Disposition

The complete directed segment-register batch is verified on the current
Release Core library.  The Model-40 replay remains a separate scheduler
receiver and does not qualify or invalidate this CPU result.
