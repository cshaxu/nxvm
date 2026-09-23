# T512 S5 8086/8088 Current-Owner Audit

`M5:T512:S5:I86-I88-OWNER-AUDIT:COMPLETE`

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
| Successful timing selection | `core_machine_cpu_timing_select()` | The 8088 branch is one ordered chain: string/I-O, external-L2 Group-3 arithmetic, primary, control/stack, then explicit compatibility. |
| Manual 8088 transfer adjustment | Existing transfer-plan helpers inside the string/I-O, primary and control/stack timing owners | Each adds the original table's four clocks per stated word transfer at the owner; no mutable 8088 bus or profile state is introduced. |
| LOCK | `core_machine_cpu_timing_apply_8086_lock()` after successful selection | One two-clock table-prefix adjustment; it does not retire separately or create a second decoder rule. |

The old 8086 manifest runner executes 1,053 actual 8086 retirements. Its
four currently registered checks pass, including the generated timing-result
and decoder-ledger gates. It is therefore useful current 8086 proof, not
proof that the distinct 8088 transfer arithmetic has been re-executed across
the same finite recipe universe.

## Complete-Sweep Repair And Proof

The derived 8088 manifest expands the same 1,053 decoder-admitted recipes as
8086 under `I88-` keys; one shared runner executes both profiles. The CTest
result contract independently checks Table 2-21's `base + 4*T` rule from the
8086 result and the manual's explicit transfer counts. It rejects both
`Compatibility` ownership and `source_timing_unallocated` for every 8088 row.

The sweep found and repaired two real Core gaps: fixed rows with LOCK or a
segment override were bypassing Primary, and `WAIT` plus the retained
operand-dependent Group-3 rows were falling through to Compatibility. `WAIT`
now has its manual `3+5n` Primary row. Group-3 remains the existing explicit
86Box-derived L2 model, constrained by Intel's printed ranges; its 8088 memory
word form applies the manual four-clock transfer term. No L3 row was lowered.
The completed 8086/8088 sweep leaves no L1 timing disposition: all 1,053 8088
rows are Manual-L3 or the declared Group-3 External-L2 model. Any future
five-profile S must report an unupgradeable L1 or a demonstrated correction to
an earlier tier before it can close.

The two runner executions and their two result contracts pass for 1,053/1,053
rows each. This is instruction-time evidence only: it makes no prefetch,
READY, HOLD, device-service or host-wall-clock claim.

`M5:T512:S5:I86-I88-ONE-TEST-OWNER:COMPLETE`
