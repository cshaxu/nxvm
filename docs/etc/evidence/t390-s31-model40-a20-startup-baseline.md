# T390 S31: Model-40 A20 Startup Baseline

## Outcome

The Model-40 A20-disabled physical-address policy previously cleared A20 for
all 32-bit physical addresses.  That folded the 80386 reset vector and the
high ROM aliases below their registered routes.  The selected Model-40 policy
now mirrors only the second MiB into the first MiB while A20 is disabled;
addresses outside that compatibility window, including the high reset and ROM
aliases, remain physical.

Focused owned regressions prove the Model-40 ROM aliases with A20 initially
disabled, the second-MiB compatibility mirror, the real KBC output-port A20
toggle path, and the unchanged generic global-mask behavior.  The policy is
owned by Core as an additive checked machine configuration; Model-40
composition selects it and does not access Core RAM state or routing internals.

## Contained formal-asset replay

The existing BYOB capture helper ran only against owner-managed external
assets under a 45-second wall-clock cap.  Its output was retained only in an
ignored temporary file, reduced to the aggregate summary below, then deleted.
No asset locator, digest, provenance, ROM/media bytes, PC, trace, or terminal
byte record is retained here.

The replay completed within the wall-clock bound with 2,000,000 successful
classified retirements, zero source-unallocated observations, 104 aggregate
forms, protected-mode observed, and the protected-to-real checkpoint observed.
The C1 diagnostic stopped only at its retirement containment budget; it did
not establish a later C1 semantic checkpoint, a timing value, physical timing,
or L3 readiness.

## Similar-issue sweep

The A20 wrapper has a single Core owner used by physical, real-mode, provider,
and route-resolution accesses.  Both Model-40 constructors select the
profile-specific policy; all other configurations retain the zero/default
`global-mask` policy.  Focused generic and Model-40 regressions cover both
dispositions.  No separate A20 masking implementation or profile-specific
RAM-route bypass was found.

The full current-source CTest gate passes 284/284 after the stale Model-40 D4 regression moved from its pre-S29 `E0000` ROM expectation to the accepted `F8000` primary and `F0000` compatibility windows.

## Transfer

A future T390 C1 receiver may use this now-meaningful startup path to define a
finite C1 checkpoint corpus.  It must retain the external-asset boundary and
must not infer CPU timing, board timing, physical retirement qualification, or
L3 closure from this containment result.
