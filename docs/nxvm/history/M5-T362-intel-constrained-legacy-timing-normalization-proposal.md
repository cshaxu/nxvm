# Intel-Constrained Legacy Timing-Model Normalization

## Purpose

Replace the temporary one-tick `reference-exhausted` timing route for the
remaining 8086/80186 dynamic-arithmetic forms with a reproducible,
source-labelled CPU timing-model allocation.  This task consumes T361's
Intel-table, 86Box, MAME, and PCjs audit.  It does not turn an external model
into a physical-hardware claim: it makes the selected model obey the applicable
Intel timing domain and makes every adjustment visible.

For a same-profile, same-form reference scalar `r` and an Intel closed timing
domain `[lo, hi]`, the model allocation is:

```
selected = min(max(r, lo), hi)
```

The ledger must state the raw scalar and the resulting value.  A value that
already lies in the Intel domain is a direct model allocation.  A value outside
that domain is an **Intel-constrained model allocation**, not an Intel-exact
or measured timing result.  For example, an i80186 `69 /r iw` source scalar of
25 against Intel Table 1-16's 29--32 range normalizes to 29.

## Scope

- T361's 8086 `F6`/`F7` `MUL` and `IMUL` forms, and 80186 `F6`/`F7`
  `MUL`/`IMUL`/`DIV`/`IDIV` plus `69`/`6B` immediate `IMUL` forms.
- Register versus memory, byte versus word, documented EA/segment-override
  treatment, odd-word handling, profile rejection, divide fault rollback,
  and the single successful-retirement tick publisher.
- A profile-local CPU timing-model representation or table that records the
  provenance class needed by the selected value; the PC/AT machine profile
  consumes that CPU result and remains the owner of bus, wait, and device time.
- A complete cleanup sweep of the scoped forms' `UNALLOCATED`/one-tick fallback
  paths and tests, leaving an explicit receiver only where no comparable
  source and Intel domain exist.

## Normalization Preconditions

Each candidate row is eligible only when the evidence ledger establishes all
of the following before implementation:

1. the Intel row applies to the exact CPU profile, opcode/form, width, and
   register/memory domain being allocated;
2. the reference value is from a documented same-profile implementation and
   exact upstream revision/path;
3. the comparison has one accounting basis: whether the value includes EA,
   segment override, odd-word transfer, prefetch, bus waits, and device time;
4. only CPU instruction clocks are normalized.  Machine/bus/device additions
   remain separate owners and must never be clamped into an instruction row;
5. the raw source scalar, Intel bounds, selected value, and provenance class
   are recorded in durable evidence and asserted by a focused smoke.

MAME i86/i186 is the first selected scalar source under the owner-approved
T361 order.  86Box may be used only if its cycle model can be separated from
bus/prefetch ownership without importing code.  PCjs is an 8086 candidate
only after excluding its explicitly benchmark-calibrated values.  QEMU,
Bochs, and PC110-EMU remain functional/event references unless a later audit
finds an exact profile-local CPU-clock row.

## Non-goals And Stop Conditions

Do not choose a midpoint, silently replace an unsupported form with one tick,
borrow another CPU generation, import third-party code, measure host time,
implement physical bus/prefetch timing, or call the result cycle exact.
Do not normalize an EA-included source scalar against a register-only Intel
range.  Stop a row and retain a precise `reference-exhausted` transfer if its
form domain, accounting basis, or source revision cannot be proven.

This task does not reopen T361's execution-round mechanism, alter exception
delivery/vector/frame semantics, or allocate 80286/80386 timing.  It also does
not change the selected PC/AT profile's machine-level timing policy.

## Required Evidence And Verification

- A form-by-form ledger with Intel source/page/row, inclusive bounds, raw
  model scalar and revision/path, accounting basis, normalized result, and
  direct/clamped/reference-exhausted disposition.
- Tests for direct-in-range values, both lower and upper clamp boundaries,
  register/memory/width/prefix variants, no double EA/odd-word/segment charge,
  successful retirement, divide fault nonpublication, and profile rejection.
- A source/consumer sweep proving that each scoped form has exactly one CPU
  timing allocation and no residual one-tick fallback when an eligible model
  row exists.
- Documentation governance, diff check, focused timing smokes, and full
  current-gate.  The closure audit must name every retained
  `reference-exhausted` row and state why it cannot be normalized.

## Ordered Subtasks

1. **S1 -- source-domain and normalization ledger.** Reconcile Intel rows,
   model scalar domains, EA/prefix ownership, and the complete fallback sweep
   before changing runtime values.
2. **S2 -- profile-local allocation and regression corpus.** Implement the
   ledger as a centralized model allocation, remove qualifying one-tick
   fallbacks, and prove the timing/publication boundaries.
3. **S3 -- closure audit.** Independently review every scoped form, residual
   transfer, profile/machine ownership boundary, documentation, and full gate.

## Completion Standard

This candidate closes only when every T361-transferred dynamic-arithmetic form is either
an Intel-exact, direct model-derived, Intel-constrained model-derived, or
precisely reference-exhausted row; all qualifying one-tick fallbacks are gone;
the CPU timing-model versus machine-profile boundary is explicit; and the
evidence and full current gate prove the result.  It contributes to model-L3
timing only and never to a physical cycle-exact claim.
