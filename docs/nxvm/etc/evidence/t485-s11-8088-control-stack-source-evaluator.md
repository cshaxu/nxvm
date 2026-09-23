# T485 S11 8088 Control/Stack Source Evaluator

`M5:T485:S11:8088-CONTROL-STACK-SOURCE-EVALUATOR:ACCEPTED`

The existing `core_machine_control_stack_source_instruction_cost()` remains
the one decoder-derived control/stack receiver.  The 8088 selector now reaches
it after the existing primary receiver and before compatibility.  It consumes
one private, immediate `core_machine_source_transfer_plan`; no CPU, bus or
profile state was added.

## Source equation and covered forms

The rendered 1981 Intel Table 2-21 rule is exact: `result = base + 4 * word
transfers`.  The local plan supplies its source-complete transfer count for
CALL/JMP/RET/PUSH/POP/PUSHF/POPF only: `0`, `1`, `2`, or `4` as recorded by
S10.  Existing 8086-family EA and segment-prefix computation remains the sole
memory modifier; on 8088 it supplies EA/prefix only, leaving every word
transfer to the table rule.  The repaired 8086/8088 FAR-RET gate now selects
the Table-2-20 bases `18` (RETF) and `17` (RETF imm16), then Table-2-21 adds
eight clocks.

`INT3`, `INT imm`, `INTO`, `IRET`, and `HLT` are deliberately marked
source-unallocated for 8088 by that same evaluator.  This task does not claim
delivery, vectoring, flags, halting, prefetch, external cycles or a physical
time axis.

## Focused proof

The focused retirement smoke passes after an outside-sandbox rebuild of its
actual executable:

```text
cmake --build build/mingw-gcc-x64 --target core-machine-retirement-observation-s3-smoke -- -j 1
ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R "^current\.core-machine-retirement-observation-s3-smoke$"
1/1 Test #49: current.core-machine-retirement-observation-s3-smoke ... Passed
```

It asserts the retained control/stack origin and exact 8088 ticks for register
PUSH/POP (`15`/`12`), near CALL/RET (`23`/`12`), FAR RET (`26`), and direct
memory CALL/PUSH/POP (`35`/`30`/`31`); it also proves INT3 and HLT remain
source-unallocated.

The rebuilt 299-test current gate also passes: `299/299`, `162.89 sec` test
time (`164.81 sec` wall time).  The 8086 timing-manifest runner was included
in that run and passed after the FAR-RET repair was narrowed to 8088, preserving
the existing 8086 receiver.

## Minimalism accounting

The implementation replaces repeated local lookup/add/next sequencing with
one owner-local result helper.  It adds one finite plan, one 8088 selector
position and focused assertions; it creates no public ABI, decoder, timing
state, profile setting, compatibility path or mutable mirror.
