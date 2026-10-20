# M1 T4 S26 — Tilemap DMA Context Attribution

## Boundary

S25 moved the selected owner-set startup path beyond mode `0x0015`.  The next
rejected trigger is raw mode `0x0014`.  Its ordinary source/size/destination
fields were already available from the COP-owned bounded failure record, but
the related `0x476` configuration word was not.  S26 records that one existing
word at the same first-rejection owner; it does not create bus readback or a
trace facility.

## Observation

Both x64 and x86 selected-set probes report:

```text
cop-operation=2 cop-value=20 cop-source=831 cop-size=635
cop-destination=65534 cop-relative=0 cop-physical=1788 cop-status=2
executed=87920 ticks=1401690
```

The finite selected descriptor is therefore mode `0x0014`, source `0x033f`,
size `0x027b`, destination `0xfffe`, relative `0`, trigger `0x06fc`, and the
existing invalid-state outcome.  The zero relative word rules out treating
that configuration word as an unobserved nonzero base for this selected path.
It does not, by itself, establish a length formula, destination meaning,
tile format, screen layout, timing or frame publication.

## Local contract and verification

The existing COP failure observation is still the single reset-cleared owner.
Its new field is populated only for an immediate unsupported DMA trigger, at
the same time as the existing selected context fields.  Existing Board smoke
covers capture and reset-zero behavior with a synthetic `0xdead` relative
word.  The mode remains rejected and creates no pending operation.

No Shared component, product artifact, ROM/media payload or playable-game
claim changed.  The next possible tilemap receiver must separately state its
finite L2 length and private Video ownership before implementation.
