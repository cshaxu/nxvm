# T388 S5: 80386 Jcc Target Lexeme

`M5:T388:S5:JCC-TARGET-LEXEME:OK`

## Core Boundary

The existing private Core preview is sufficient: after a successfully executed
taken 80386 `Jcc`, `core_machine_cpu_execution_preview_lexeme()` scans the
current target `EIP` through copied CPU/instruction state with preview mode.
It clears transaction, trace and diagnostic providers, then derives only the
next target instruction's lexical `component_count`. The 80386 timing owner
uses that count with `CORE_MACHINE_80386_JCC_TAKEN_TICKS` for the Intel PRM section 17.2.2.3 `m` term. It does not
execute the target or publish an elapsed tick itself.

A preview failure or unavailable target reaches
`core_machine_source_timing_mark_unallocated()` as the explicit successful
unallocated route; in the typed physical contract S3 rejects it before any
result/timeline/device/provider publication. No profile is enabled by this S.

## Regression

`core-machine-cpu-timing-preview-smoke` now executes a taken near `Jcc` in a
32-bit code segment (`0F 85` with a 32-bit self target), then proves its target
lexeme is available, has the expected six-byte/three-component layout, and
leaves the complete machine observation unchanged. The existing short-loop
case retains the 16-bit-code short branch route. The unavailable/limited fetch
case retains failure-safe, nonpublishing preview behavior.

`core-machine-instruction-timing-smoke` exercises the physical contract with
an 80386 taken short `Jcc`: the source-backed `7 + m` result publishes nine
ticks. The same contract rejects a successful unallocated prefixed path before
execution/result/elapsed publication. The direct external advance remains
invalid in physical mode.

## Boundary

This proves only source-form lexical availability for admitted 80386 `Jcc`
forms. Prefix/default breadth, protected translation failures, 80286
Appendix-B capture, physical board-clock enablement and all L3 conclusions
remain separate receivers.