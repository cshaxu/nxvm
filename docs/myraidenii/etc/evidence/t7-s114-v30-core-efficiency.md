# M1 T7 S114 — V30/Core execution-efficiency evidence

## Scope

This record covers one Shared X86 Core retirement-path cleanup.  It does not
change the guest clock, instruction semantics, scheduler order, timing model,
audio, video, input, public API, or opaque layout.

## Reproduction baseline

The existing selected-set startup probe was run through the verified external
`O:\assets\raidenii\raiden2.zip` staging wrapper at an
8,000,000-instruction budget.  The accepted S113 baseline completed
8,000,000 retired instructions and 133,667,500 guest ticks in 23,650.1 ms,
with the following observable checkpoint:

- frame digest: `0364633fdce1196d`;
- 463 VBlank events and 73,584 non-black pixels;
- 100,444 non-zero generated PCM samples;
- 4,948 YM address writes and 4,948 YM data writes; and
- four active OKI voices with no Board or COP provider failure.

## Removed work and retained owner

`x86_core_machine_execute_budget()` previously copied the retirement
eligibility key after every successful instruction.  That copy is consumed
only by physical-retirement qualification or by an installed retirement
observation provider.  A deterministic machine without a provider is already
qualified by its declared contract, so the copy cannot affect any later
decision.

The same hot path also called retirement observation publication when no
provider existed.  That call immediately returned and its result was unused.

The sole retained path is therefore:

```
physical retirement contract OR installed observation provider
    -> capture eligibility key
installed observation provider
    -> publish observation
```

Physical qualification remains unchanged.  A first narrower condition exposed
that installed observers need the captured key even under a deterministic
contract; the final condition preserves that public observation behavior.

## Result

Two post-change x64 executions used the same archive, budget and checkpoint:

| Run | Wall time | Guest facts |
| --- | ---: | --- |
| 1 | 19,536 ms | identical baseline checkpoint |
| 2 | 18,359 ms | identical baseline checkpoint |

Against the 23,650.1 ms baseline this is a 17–22% reduction for this bounded
host measurement.  It is not a real-time gameplay claim: the remaining
execution cost is still material and needs a separately designed CPU-engine
task rather than further speculative micro-patches.

The identical 8,000,000-instruction selected-set route also completed on x86.
It reached 133,667,500 ticks with the same terminal frame digest
`0364633fdce1196d`, 463 VBlanks, 73,584 non-black pixels, 100,444 generated
non-zero PCM samples, 4,948 YM address/data writes, four active OKI voices,
and no Board or COP provider failure.  The x86 probe is an interoperability
check, not a wall-clock comparison.

## Similar-path review

- `retirement_eligibility_key` is read only by physical qualification; its
  existing physical capture remains intact.
- Existing observer tests require captured identity when a provider is
  installed; the final branch preserves it.
- No duplicate scheduler, attachment, frame, audio, or Board route was
  removed because each still performs a distinct guest-visible function.

## Verification boundary

Focused X86 Core tests passed on x64 and x86.  The complete repository-only
unit suite passed on both widths (697/697 each).  All eight PC receiver EXEs
and both MyRaidenII EXEs were rebuilt and their PE identities checked.  No
desktop, RDP, or audible-output qualification is claimed.
