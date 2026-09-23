# T490 S5 Retirement-Observation Gate Correction

`M5:T490:S5:RETIREMENT-OBSERVATION-ORACLE:CORRECTED`

The S4 full gate failed before the 8253 implementation and on its clean
pre-S4 baseline. The failure was in
`retirement_unallocated_profile_case`: it requested `REPEAT` for the 8088
`MUL AL` (`F6 E0`) and `WAIT` (`9B`) inputs, although neither byte sequence
contains a `F2` or `F3` REP prefix. The existing Core formula-input owner
correctly left `CORE_MACHINE_CPU_TIMING_INPUT_REPEAT` clear. S5 changes only
the smoke oracle: its `expected_repeat` argument now checks both presence and
absence, and those two unprefixed calls pass `0`. Existing `F3 90` cases retain
the positive REP assertion across every CPU profile.

## Ownership and sweep

The production sweep was limited to all tracked CPU timing and
retirement-observation construction/capture/publish sites:

```text
rg -n -C 3 "source_timing_formula_inputs|source_timing_repeat_phase|prefix_rep|cpu_timing_formula_inputs" src/core/machine
rg -n -C 2 "rep_nop|mul_al|wait\[|capture_instruction\(|capture_eligibility|cpu_timing_apply|source_timing_repeat_phase" tests/core/machine_retirement_observation_s3_smoke.c src/core/machine --glob '!cpu_instructions.c'
```

`cpu_timing.c` is the sole formula-input producer; the retirement interface
only captures/publishes that completed result. No production hit needed a
change, and no second prefix, timing, or retirement state path was added.

## Verification

- Rebuilt `core-machine-retirement-observation-s3-smoke` outside the restricted
  Ninja sandbox after two owned, stuck Ninja processes were stopped.
- Focused CTest selection: 6/6 passed (`retirement-observation`, 8253,
  8254 Read-Back, waveform, IRQ0 and XT profile).
- Full configured CTest: 300/300 passed; the original test 49 and all retained
  PIT/XT regressions passed.
- `tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`: passed.
