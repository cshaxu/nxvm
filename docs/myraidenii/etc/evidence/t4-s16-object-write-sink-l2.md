# M1 T4 S16 — adjacent object write sink L2

## Evidence boundary

S15 established the exact `0x658--0x659` write-only span and the selected-set
probe then stopped on a one-byte write at `0x65a`. FBNeo's CRTC path ends at
`0x64f`; MAME labels the surrounding `0x640--0x6bf` region as an OBJ
interface, but its mapping is commented out. Neither source defines object
register contents, reads or algorithms.

## Model

The Board owns a second stateless write-only L2 pair at precisely
`0x65a--0x65b`. A single four-byte Board route prevents a cross-pair write
from falling through into ordinary RAM. Its write callback accepts only an
access contained by either exact two-byte pair; reads, inspection, the
`0x659--0x65a` cross-pair and neighbours remain refused. The model retains no
guest payload and has no renderer, CRTC, palette or frame effect.

## Result

Both host widths pass the focused Board test. The owner-supplied selected-set
probe advances identically to the next first fallback write at decimal `1628`
(`0x65c`):

```text
control-valid=1 control-physical=1628 control-bytes=1 control-access=1
executed=71422 ticks=1132340
```

This remains an L2 address-ownership disposition, not a physical object
hardware claim or playable-game qualification.
