# M1 T4 S24 — COP DMA Context Attribution

## Basis and boundary

S23 proved that the selected owner-set execution reaches the existing COP DMA
trigger at `0x6fc` with rejected raw mode `0x0015`.  The mode's selected context
is intentionally write-only on the Board bus, so reading it back through a
diagnostic route would weaken the modeled device contract.

S24 instead extends the existing first-failure observation at the COP owner.
It snapshots only the already-selected context's three 16-bit descriptor
values when that existing rejection occurs.  It neither retains ROM bytes nor
instruction addresses, accepts the mode, creates a trace facility, nor exposes
the context through normal Board reads.

## Observed descriptor

The bounded owner-set startup probe produces the identical result on x64 and
x86:

```text
cop-operation=2 cop-value=21 cop-source=1984 cop-size=255 cop-destination=0
cop-physical=1788 cop-status=2
```

Thus the rejected mode is `0x0015`, with selected context source `0x07c0`,
size `0x00ff`, destination `0x0000`, trigger address `0x06fc`, and the existing
`LIB_STATUS_INVALID_STATE` result.  These are a bounded configuration
observation only; they do not establish byte-count interpretation, palette
word format, destination semantics, transfer timing, reset behavior beyond the
existing context reset, or any Video publication behavior.

## Local contract

`myraidenii_core_cop_failure_observation` remains the one COP-owned,
reset-cleared first-failure record.  A rejected DMA trigger stores its selected
descriptor if its mode identifies one of the existing 512 contexts; an
out-of-range rejected mode still records the original mode/status/address and
leaves descriptor fields zero.  Board's existing getter and the existing
startup probe consume that one record.  The DMA context remains write-only
through the real bus.

## Verification

The existing Board smoke configures an unsupported context, confirms all three
recorded fields, proves a later supported trigger is still accepted (so the
rejection leaves no pending operation), and verifies reset clears the complete
record.  Existing accepted-DMA failure coverage stays in the same test.

Focused Board smokes and selected-set probes pass on x64 and x86.  This result
does not admit mode `0x0015`, a palette receiver, Video DMA, a frame, or a
playable-game claim.  It narrows the next evidence question to the meaning of
the observed descriptor.
