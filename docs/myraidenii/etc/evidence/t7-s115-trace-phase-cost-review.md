# M1 T7 S115 — Transaction trace-phase cost review

## Scope

This bounded review examined the Release transaction trace route in Shared X86
Core while running the existing selected RAIDEN II set.  No production change
is retained.  The external archive remained read-only and no payload, staged
image or raw trace is retained.

## Finding

The first candidate appeared to be fully diagnostic: every generic transaction
phase invokes `x86_core_machine_transaction_trace()`, whose final publication
is compiled out in Release.  That conclusion was incomplete.  The callback
also performs two required Machine operations:

- `BEGIN` clears `bus_invalidation_seen`; and
- an acknowledged DMA `HOLD` invalidates the external-cycle locality state.

Removing the callback entirely made the existing prefetch-locality regression
fail on both widths.  This rejects a trace-only interpretation and rules out
a dynamic bind/unbind layer as an optimization: that layer would either lose
the two physical operations or add a second ownership path.

## Bounded Release experiment

The candidate retained those two physical phases and skipped the remaining
Release-only diagnostic phases.  It passed the complete X86 Core unit
selection on both widths (196/196 each), and the selected-set checkpoint was
unchanged:

- 8,000,000 instructions;
- 131,798,219 guest ticks;
- terminal frame digest `a388337a0d6ec5df`;
- no Board or COP provider failure.

The measured x64 route took 19,512.9 ms.  This is within the accepted route's
host-time variation and does not establish a durable improvement.  The x86
route completed with the same checkpoint in 23,451.1 ms; it is an
interoperability result, not a comparable host-speed result.

The trial source was restored.  There is therefore no source, ABI, artifact,
or test change from this S.

## Disposition and similar-path sweep

Retirement observation had already been made conditional in S114.  The
remaining transaction callback is not a duplicate owner: it combines one
physical bus-locality owner with compile-time-disabled diagnostic publication.
Splitting it would introduce a new contract for a non-measurable gain.

No scheduler, attachment deadline, CPU event/result publication, Board time
dispatch, frame, audio, timing-model or guest-clock operation was removed or
changed.  The declared no-durable-result stop condition applies; later work
must begin from an independently measured CPU/Core cost rather than extend
this rejected micro-optimization.
