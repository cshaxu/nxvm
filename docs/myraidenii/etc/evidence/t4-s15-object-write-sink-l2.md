# M1 T4 S15 — object write sink L2

## Evidence boundary

S14's bounded diagnostic proved that the selected set first writes byte lanes
at `0x658` and `0x659`. FBNeo's CRTC path ends at `0x64f`; MAME labels the
surrounding `0x640--0x6bf` region as an OBJ interface, but its mapping is
commented out. These sources establish neither object register meanings nor
readback behavior.

## Model

The Board therefore owns one stateless, write-only L2 sink at precisely
`0x658--0x659`. It accepts only contained byte writes and the contained word
through the existing Core route. Reads, inspection, mixed ranges and adjacent
addresses remain refused. The sink does not read or retain guest data and has
no renderer, CRTC, palette or frame effect.

## Result

Both host widths pass the focused Board test. The owner-supplied selected-set
probe advances identically from the S14 fault at `0x658` to the next first
fallback write at decimal `1626` (`0x65a`):

```text
control-valid=1 control-physical=1626 control-bytes=1 control-access=1
executed=71413 ticks=1132235
```

This is an L2 address-ownership disposition, not an assertion of physical
object hardware behavior or a playable-game qualification.
