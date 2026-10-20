# M1 T4 S9 Selected Control Hole L2 Record

## Decision

S9 admits only the two-byte write-only, stateless Board route at
`0x690--0x691`.  It has no stored state, readback or inspection surface.  It
does not convert the surrounding `0x690--0x69f` region into generic RAM or a
generic accepted control range.

The owner authorized bounded L2 behavioral models where L3 material is absent.
The current reference map's unassigned local interval is L2 context only; no
external implementation, table or protected payload was imported.

## Boundary Proof

The selected program stopped at `0x690` on both host widths after accepted
S8.  The existing Board smoke proves byte writes at both lanes and contained
word writes.  It rejects reads and inspection.  A word starting at `0x691` is
accepted by the pre-existing Core byte-route splitter because its two lanes are
already independently accepted (`0x691` and the prior route at `0x692`);
access including an unknown lane remains rejected.

This preserves the S7 decision: a broad `0x690--0x69f` fallback changed
neighbour failure behavior and remains prohibited.

## Selected-Set Result And Transfer

Both bounded replays advance past `0x690` identically, then terminate with:

| Host width | Stop reason | Detail | CPU fault code | Executed instructions | Ticks |
| --- | --- | ---: | ---: | ---: | ---: |
| x64 | fault | `LIB_STATUS_INVALID_STATE` | none | 68,255 | 1,076,837 |
| x86 | fault | `LIB_STATUS_INVALID_STATE` | none | 68,255 | 1,076,837 |

`detail=2` is the existing `LIB_STATUS_INVALID_STATE` value.  The CPU
diagnostic has no first fault code.  The only Board path that reports this
status is its attachment's existing `myraidenii_core_cop_advance()` result
propagation.  The stop is therefore transferred to COP command/receiver
qualification.  It is not evidence for another Board address hole.

The runner transiently staged only verified program lanes and removed its
generated child directory.  This proves only bounded forward progress and a
COP-owned next failure boundary; it does not prove a physical contract,
complete startup, video, sound or a playable game.

## Scope

Only MyRaidenII Board source, its existing unit executable and task records
changed.  No Shared/public API, ROM/media, release executable or external
source changed.
