# T582 S36 — CPU hot path investigation

## 486 timing evaluator flatten experiment

A scratch-only GCC experiment marked the existing 486 source timing evaluator
`flatten`, allowing the compiler to inline its internal callees without
changing the runtime executor, source formulas or component boundaries. The
production build did not receive the compiler-specific attribute. With
baseline/candidate/candidate/baseline order at the private PC110 startup
checkpoint, elapsed milliseconds were 20,516 / 18,007 / 20,396 / 18,458 on
x64 and 22,084 / 23,650 / 23,829 / 24,004 on x86. Every run reported ten
million instructions, 79,964,294 guest ticks and PC `0003dbc5`. This endpoint
is not a full state-equivalence test, Game performance or desktop use.

Both widths changed direction between ordered comparisons; candidate EXE size
increased from 3,092,997 to 3,120,133 bytes x64 and from 3,256,457 to
3,294,345 bytes x86. The samples do not establish a reproducible benefit to
justify increased machine code and compiler-specific build behavior, so the
candidate was discarded without changing production. Combined log SHA-256:
`D130F02607849D85D4F9FA61F9D010976285FCAB2B273F6CC3B122290AE96AC8`.
