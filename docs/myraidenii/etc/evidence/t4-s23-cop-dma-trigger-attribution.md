# M1 T4 S23 — COP DMA Trigger Attribution

## Basis and boundary

S22 advanced the selected-set probe to the existing COP DMA trigger at
`0x6fc--0x6fd`.  The COP already owned that pair and already rejected a DMA
mode outside its admitted local model.  Its failure observation, however, was
only populated after an accepted pending operation reached `advance()`.  An
immediate trigger rejection therefore reached the CPU fault path without the
existing diagnostic exposing its mode.

This change records that already-existing rejection through the existing
bounded COP observation.  It does not admit a mode, infer a DMA receiver,
inspect ROM bytes, retain a guest trace, or establish hardware timing.

## Local model

COP is the sole owner of its failure observation.  Board no longer maintains a
second snapshot; its existing getter delegates to COP.  On the first rejected
DMA trigger, COP records the existing operation, raw mode, physical address
and `LIB_STATUS_INVALID_STATE`, then leaves the pending operation empty and
returns the same status as before.  Reset clears the existing observation.

The existing selected-set probes now report the same first rejected trigger on
both widths:

```text
cop-operation=2 cop-value=21 cop-physical=1788 cop-status=2
```

That is DMA operation, raw mode `0x0015`, physical address `0x6fc`, and the
existing invalid-state status.  It is attribution only: mode `0x0015` remains
unimplemented and rejected.

## Verification

The existing Board smoke verifies an unsupported `0x0014` trigger records the
same four bounded fields through the Board getter, leaves no pending operation
and clears on reset.  Existing accepted-DMA failure coverage remains in the
same smoke.

Focused x64/x86 Board smokes pass.  The owner-supplied selected-set probe
passes on both widths and produces the identical attribution above.  No Shared
component, public API, product artifact, or protected media changed.
