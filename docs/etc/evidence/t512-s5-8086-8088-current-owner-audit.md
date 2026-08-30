# T512 S5 8086/8088 Current-Owner Audit

`M5:T512:S5:I86-I88-OWNER-AUDIT:COMPLETE-PENDING-IMPLEMENTATION`

## Manual-First Check

Intel *iAPX 86,88 User's Manual* (1981), Table 2-21, printed pages 2-51
through 2-68, is the semantic and instruction-time authority for both CPUs.
The owner-managed scan was rendered for this review. Its repeated footnote
states that the 8088 adds four clocks for each listed 16-bit word transfer;
this is a Manual-L3 per-row adjustment, not an 8086-derived bus estimate.
The table also marks INTR, NMI and single-step as non-instructions.

## Current Sole-Owner Trace

| Boundary | Current path | Audit result |
| --- | --- | --- |
| Decode, execution, state and delivery | `cpu_instructions.c`, followed by the existing `ExecFinal()`/`ExecInt()` paths | One shared 8086-class Core path; no VM/profile execution route was found. |
| Successful timing selection | `core_machine_cpu_timing_select()` | The 8088 branch is one ordered chain: string/I-O, primary, control/stack, then explicit compatibility. |
| Manual 8088 transfer adjustment | Existing transfer-plan helpers inside the string/I-O, primary and control/stack timing owners | Each adds the original table's four clocks per stated word transfer at the owner; no mutable 8088 bus or profile state is introduced. |
| LOCK | `core_machine_cpu_timing_apply_8086_lock()` after successful selection | One two-clock table-prefix adjustment; it does not retire separately or create a second decoder rule. |

The old 8086 manifest runner executes 1,053 actual 8086 retirements. Its
four currently registered checks pass, including the generated timing-result
and decoder-ledger gates. It is therefore useful current 8086 proof, not
proof that the distinct 8088 transfer arithmetic has been re-executed across
the same finite recipe universe.

## Complete-Sweep Gap And Receiver

Earlier T486 evidence established the 8088 source/List-1/List-2 and repaired
the former missing selector cases, but its retained focused smoke is not the
same complete real-retirement matrix as the current 8086 runner. The current
metadata catalog has only `profile: "8086"`; no second checked 8088 manifest
or full 8088 result gate exists. This is a verification gap, not a basis for a
new production timing path or an inferred hardware-time claim.

S5's one implementation batch is consequently test-owner work: generalize the
existing manifest data and runner so one table-driven executor covers both
profiles, with every 8088 expected value independently expressed as the
manual base plus its explicit transfer term. It must retain the existing Core
selection chain, preserve source-unallocated Group-3/WAIT/non-string-REP and
non-instruction boundaries, and delete any superseded 8086-only test-only
assumption. A second copied opcode runner is prohibited.

`M5:T512:S5:I86-I88-ONE-TEST-OWNER:READY`
