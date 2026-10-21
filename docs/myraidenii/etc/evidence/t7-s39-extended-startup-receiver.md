# M1 T7 S39 — Extended Startup Receiver

The selected-set Probe's former 20,000,000-instruction cap was a diagnostic
limit, not a Board or ROM constraint.  S39 raises only the explicitly bounded
maximum to 100,000,000 instructions; the default remains unchanged.

With the existing vblank Coin/Start sequence, both x64 and x86 stop at the
same newly visible finite receiver:

```text
reason=7 fault-code=1280 fault-mask=2147483648
cop-operation=1 cop-value=15280 (0x3bb0)
executed=24517538 ticks=400779539
frame-digest=370d5479380a307a nonblack=70104
```

`cop-operation=1` is the existing Command classification.  This evidence
therefore identifies command `0x3bb0` as the next finite COP receiver.  It is
not evidence for its operands, memory effects, result, timing, or a safe
replacement behavior.  The next implementation packet must first establish
those semantics at L3 or bounded L2 without importing source/tables.

The new budget has not claimed an attract loop, title screen, gameplay or
release.  Owner-media staging was empty after both runs.
