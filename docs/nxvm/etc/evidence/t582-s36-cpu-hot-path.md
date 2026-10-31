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

## CPU unity compilation experiment

A scratch translation unit included the existing five CPU source files once,
allowing ordinary GCC `-O3` cross-file optimization without LTO or an alternate
executor. Compilation with the existing narrow instruction-source warning
exceptions produced no diagnostics. This was not a production CMake change;
successful compilation alone does not establish all macro, static-symbol,
standalone-component or Debug-build compatibility.

At the private fixed PC110 startup checkpoint, ordered baseline/candidate/
candidate/baseline times were 18,365 / 18,816 / 18,728 / 21,554 ms x64 and
23,537 / 25,123 / 22,402 / 22,011 ms x86. All reported ten million
instructions, 79,964,294 guest ticks and PC `0003dbc5`; this does not establish
complete state equivalence. x64 changes direction and both x86 comparisons
favor the baseline. The unity object took 20.73 seconds to compile x64 and
28.29 seconds x86; diagnostic links remained near one second. Candidate EXE
sizes increased by 9,509 bytes x64 and 5,557 bytes x86. These samples do not
justify changing the component build; Game performance, full CPU behavior and
desktop/RDP were not tested. No production source or asset changed. Combined
log SHA-256:
`BC611435F784F9E7872C971ADD59D55E3EFE3D254ABD8D276F713DCC0C80B63F`.
