# T485 S5 8088 Transfer-Plan Receiver

`M5:T485:S5:8088-TRANSFER-PLAN:BLOCKED`

## Result

The proposed direct evaluator is not lawful yet.  Intel Table 2-21 supplies
per-form `Transfers` and the 8088's four-clock-per-16-bit-word rule, but the
current successful-retirement input contains only decoded operands
(`flagMem`, ModRM shape and selected arithmetic operands).  It does not retain
one complete, source-labelled word-transfer plan for the completed
instruction.

## Owner sweep

| Current facility | What it actually owns | Why it is not the required plan |
| --- | --- | --- |
| `core_machine_source_timing_primary_word_transfers()` | Partial primary-form word-transfer counts. | It intentionally covers only its primary-form subset. |
| String and control-stack timing helpers | Local source-form additions. | They independently infer transfers and do not publish a shared result. |
| `core_machine_cpu_external_cycle_trace()` | One external byte access at a time, including begin/commit/cancel and provenance. | It models actual logical byte accesses, not the manual's completed per-form 16-bit transfer count; folding it into instruction timing would also entangle waits/prefetch/board cycles. |
| `t_cpuins_data` | Decoder/operand facts for the successful instruction. | It has no complete cross-form transfer-plan field. |

Using either the local helpers or the byte trace as a global replacement would
create parallel form classification or silently turn external bus behavior
into CPU instruction timing.  Both contradict the single-owner and
no-estimate boundaries.

## Earliest receiver

The next bounded receiver is a private Core CPU source-transfer-plan builder.
It must run from the existing decoded successful-instruction facts, provide
one immutable per-retirement count of *manual 16-bit word transfers*, and be
consumed by the 8088 evaluator only after the form declares it complete.  It
must not count external-cycle callbacks, publish guest time, expose a public
ABI, or replace the independent future IBM 5160 external-cycle classification.

Until that plan exists, all 8088 source timing remains unallocated and the
S3 physical-eligibility rejection remains correct.

## Verification

Reviewed the complete 8088 selector branch, the primary/string/control timing
helpers, `t_cpuins_data`, and the Core external-cycle trace.  A focused
`core-machine` rebuild after reverting the rejected global-surcharge trial
passes.  No runtime behavior is changed in this S.
