# M1 T4 S17 — second adjacent object write sink L2

## Evidence boundary

S16 established the `0x65a--0x65b` write-only pair. The selected-set probe
then stopped on a one-byte write at `0x65c`. FBNeo's CRTC path ends at `0x64f`;
MAME labels the surrounding `0x640--0x6bf` region as an OBJ interface, but its
mapping is commented out. Neither source defines object register contents,
reads or algorithms.

## Model

The existing Board route grows only through the adjacent `0x65c--0x65d` pair.
Its stateless write callback accepts an access contained by one of the three
exact two-byte pairs. The owned window prevents cross-pair writes from falling
through to ordinary RAM; reads, inspection, cross-pair writes and neighbours
remain refused. It retains no guest payload and has no renderer, CRTC, palette
or frame effect.

## Result

Both host widths pass the focused Board test. The owner-supplied selected-set
probe advances identically to the next first fallback write at decimal `1630`
(`0x65e`):

```text
control-valid=1 control-physical=1630 control-bytes=1 control-access=1
executed=71431 ticks=1132445
```

This remains an L2 address-ownership disposition, not a physical object
hardware claim or playable-game qualification.
