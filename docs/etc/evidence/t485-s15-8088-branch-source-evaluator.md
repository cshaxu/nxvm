# T485 S15 8088 Branch Source Evaluator

`M5:T485:S15:8088-BRANCH-SOURCE-EVALUATOR:ACCEPTED`

S14's rendered Intel Table 2-21 rows are consumed by the retained
`core_machine_control_stack_source_instruction_cost()` owner.  The owner reads
only the published post-retirement EIP and compares it with the sequential
16-bit IP (`old EIP + prefixes + 2`); it neither re-executes condition flags
nor derives a target.

| Source forms | Source result | Retained result path |
| --- | --- | --- |
| `70h`--`7Fh` Jcc | taken `16`; fallthrough `4` | one 8088 pre-switch range receiver |
| `E3h` JCXZ | taken `18`; fallthrough `6` | existing control/stack case receiver |
| `E2h` LOOP | taken `17`; fallthrough `5` | existing control/stack case receiver |
| `E1h` LOOPE/LOOPZ | taken `18`; fallthrough `6` | existing control/stack case receiver |
| `E0h` LOOPNE/LOOPNZ | taken `19`; fallthrough `5` | existing control/stack case receiver |

The sole private short-branch helper supplies the same published outcome to
both retained branch families.  8088 publishes the existing generic `JCC`
source form from this owner; it does not inherit the 8086-only form identity.
No decoder, table, CPU state, prefetch estimate, physical-time claim, or
compatibility path is added.

The focused retirement smoke executes all sixteen Jcc opcodes in both
outcomes, plus taken/fallthrough JCXZ, LOOP, LOOPE and LOOPNE.  Every record is
classified at the control/stack origin and has a non-unattributed source form.
The focused build/test and the complete 299/299 current gate pass at
`3ccae601`; specialized gates and documentation governance also pass.

Counted tracked code paths are `src/core/machine/cpu_timing_model.c` and
`tests/core/machine_retirement_observation_s3_smoke.c`; documentation and
generated build output are excluded.  The mechanism retains one evaluator and
one outcome calculation.  The counted code change is `+108/-5` lines
(`+31/-5` production and `+77/-0` test); the positive test coverage is the
complete finite source-form proof, not a second production path.
