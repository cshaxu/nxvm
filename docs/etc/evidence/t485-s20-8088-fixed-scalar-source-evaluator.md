# T485 S20 8088 Fixed Scalar Source Evaluator

`M5:T485:S20:8088-FIXED-SCALAR-SOURCE-EVALUATOR:ACCEPTED`

The retained private primary evaluator is the single 8088 receiver for this
fixed, unprefixed batch.  It assigns the existing private source form and
exact Intel Table 2-21 result before the generic primary-form classifier and
before compatibility:

| Opcode family | Exact 8088 clocks |
| --- | ---: |
| NOP | 3 |
| CLC, CLD, CLI, CMC, STC, STD, STI | 2 |
| LAHF, SAHF | 4 |

The focused retirement-observation smoke executes every one of the ten
opcodes.  Each record is classified at `PRIMARY`, has a non-unattributed
source form and reports the listed value.  The pre-existing `REP NOP`
assertion remains source-unallocated at `COMPATIBILITY`, proving this batch
does not claim prefix timing.

No decoder, timing-plan type, public API, mutable machine state, profile
setting, selector stage or compatibility path was added.  The private switch
is the retained production path.  XLAT, MOV/segment/pointer, Group-2, LOCK,
segment-prefix, ESC/WAIT/FPU and asynchronous delivery remain S19 transfers.

The focused build/test and `current-gates-gcc` preset pass.  Counted tracked
code paths are `cpu_timing_model.c` (`+46/-0`) and the focused smoke
(`+16/-1`), for `+62/-1` code/test lines; documentation is excluded.  The
positive change is the complete ten-opcode proof and the source-attributed
receiver, not a parallel timing mechanism.
