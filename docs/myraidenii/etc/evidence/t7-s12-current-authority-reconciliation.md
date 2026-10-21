# M1 T7 S12 — Current Authority Reconciliation

## Evidence reviewed

The current source root registers exactly these active components:

```text
src/myraidenii/core
src/myraidenii/machine
src/myraidenii/product
```

`test/myraidenii` registers matching `unit/core`, `unit/machine` and
`unit/product` roots, plus separate external integration tests.  The closed
T3, T4, T5 and T6 records confirm the finite Board/COP, native frame, local
sound and Product/Machine composition deliveries respectively.

## Corrections

- The source-root README no longer calls the current tree a T3-only,
  non-runnable Board scaffold.
- The source-layout authority no longer says Product configuration,
  composition and controls are unimplemented, or that the version header is
  merely future work.
- The architecture authority now distinguishes existing finite COP/video/sound
  mechanisms and one RGB32 publication route from the still-unqualified game
  result.
- The roadmap now identifies T7 as the active qualification task after closed
  T1--T6 rather than presenting T3--T6 as future queue work.

## Retained gaps

No wording claims that a Product process, bounded selected-set run, frame copy,
local PCM production or successful unit test qualifies gameplay.  T7 still
requires the actual startup, attract, input, stage/end/restart, sound,
lifecycle and owner-acceptance evidence.  COPX-D2 and sprite blend fidelity
remain explicit TODO receivers.  Historical proposals and records remain
unchanged because they report their former task boundaries rather than current
architecture.
