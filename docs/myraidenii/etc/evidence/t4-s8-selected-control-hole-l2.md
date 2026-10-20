# M1 T4 S8 Selected Control Hole L2 Record

## Decision

S8 admits only the two-byte write-only, stateless Board route at
`0x69a--0x69b`.  The route has no stored state, no readback and no inspection
surface.  It is not generic RAM and does not widen the surrounding
`0x690--0x69f` control interval.

This is an L2 behavioral model.  The owner authorized bounded models based on
other emulators where physical-level material is unavailable; no external
implementation, table or protected payload was imported.

## Evidence And Boundaries

After the accepted exact routes at `0x692--0x693`, `0x696--0x697` and
`0x69c--0x69d`, the selected-set diagnostic stopped identically at `0x69a` on
both host widths.  The independently consulted reference map leaves the local
`0x690--0x69f` span unassigned between named receivers.  That map fact is L2
context only, not an implementation source.

The Board unit proves byte writes at both lanes and a contained word write.
It rejects reads and inspection.  A word starting at `0x69b` is correctly
accepted by the existing Core byte-route splitter because it contains only two
already accepted lanes: `0x69b` and the pre-existing route at `0x69c`.  Access
that includes an unknown lane remains rejected; no cross-boundary exception
was added.

The former S7 experiment that widened the whole interval was rejected: it
changed established neighbouring-access failure behavior.  S8 therefore adds
only this exact route.

## Actual Selected-Set Replay

With the new route, the existing bounded diagnostic advanced past `0x69a` and
then stopped at the next unmapped address, `0x690`, on both widths:

| Host width | Stop reason | Fault code | Executed instructions | Ticks |
| --- | --- | --- | ---: | ---: |
| x64 | fault | `0x690` | 68,249 | 1,076,679 |
| x86 | fault | `0x690` | 68,249 | 1,076,679 |

The temporary runner staged only the two verified program lanes and removed
its generated child directory.  This is forward-progress evidence only; it
does not establish a physical device contract, full startup, rendering, sound
or playable-game qualification.

## Scope

Production ownership remains `src/myraidenii/core/board.c`; no Shared, public
API, ABI, ROM/media or executable artifact changed.  The existing Board smoke
received the exact acceptance and boundary regression.  The next `0x690` stop
is deliberately not absorbed by this S.
