# T582 S26 — 486 timing term sampling

An x64 task-only build instrumented the existing 486 source-cost evaluator
at one call in 64 and separately timed its nominal, L2 fallback, memory,
decode and bus term calls. The normal conditional call order and calculated
results were retained. The private-media product-path probe sent Enter using
Machine input; its sample-36 image was visually verified as World Clock. The
run completed in 372.87 host seconds. It was not a native Window/RDP test.

Between the nearest logged points to samples 30 and 36, guest ticks increased
192,453,129 over 60.668 host seconds. The sampled TSC deltas and call counts
were: nominal 65,832,249/553,481; L2 fallback 174,380/556; memory
45,145,142/553,481; decode 44,567,112/553,481; bus
23,458,756/553,481. Among these term intervals, excluding the much rarer L2
fallback, the shares are approximately 36.8%, 25.2%, 24.9% and 13.1%.
They do not include the rest of timing selection or Machine execution and
must not be interpreted as whole-program percentages.

Nested timers add overhead and influence compiler layout; deterministic
one-in-64 sampling may alias with guest loops, TSC reads are not serialized,
and preemption can inflate intervals. This establishes a direction for
further investigation, not a proven optimization or a timing formula that can
be omitted. The previously tested shared-prefix prototype did not establish
a cross-width or actual-game improvement. No production code, public API,
tests, released binaries or owner media were changed.

SHA-256 (guest log, final PPM):
`28164E15D33B9B106D0CE75EEB7E936DDA821EA97CF57C9A4FCA4DD5A8A09C6C`,
`54E10943DB0E7DCEF309B9973AB2BB667128781D9C04E7E502BA7B43A5E2D9A6`.
