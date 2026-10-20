# T4 S18 Object Aperture L2 Record

## Scope

This record closes the incremental S15--S17 object-write placeholders into one
stateless Board aperture.  It changes only MyRaidenII Board code and its
existing Board smoke; it does not change Shared components, public interfaces,
ROM contents, renderer behavior, or retained media.

## Boundary

Research-only external map comparison identifies an OBJ region beginning at
`0x640`, while the independent CRTC map evidence ends at `0x64f`.  This Board
already owns protection upload at `0x6a0--0x6bf`.  The exact unowned interval
is therefore `0x650--0x69f`.

The aperture accepts contained writes without storing payload, exposing
readback, choosing registers, decoding objects, or publishing frames.  It is
an L2 address-ownership model, not a claim of Seibu hardware semantics.

Existing selected control words inside the interval remain higher-priority
receivers.  A mixed-width request that straddles one such reserved word is
rejected by the aperture so Core does not split it into an invented object
write.  A request fully composed of adjacent already-owned control bytes still
uses Core's existing byte-route splitting.  No second dispatcher or state was
introduced.

## Sources and policy boundary

The map was used only to establish an address ownership boundary; no external
source code, tables, algorithms, ROM payload, or reference behavior was
copied.

- MAME's commented Raiden II map labels `0x640--0x6bf` as OBJ:
  <https://raw.githubusercontent.com/mamedev/mame/master/src/mame/seibu/raiden2.cpp>
- FBNeo's Raiden II CRTC writes stop at `0x64f`:
  <https://raw.githubusercontent.com/finalburnneo/FBNeo/master/src/burn/drv/pst90s/d_raiden2.cpp>

## Verification

The extended existing Board smoke proves representative aperture writes at
`0x650`, `0x658`, `0x65e`, and `0x69e`; no read/inspect path or fallback
diagnostic is accepted.  It also retains the existing selected-control
misalignment checks and moves the independent failure-observation probe to
unmodelled `0x6c0`.

Focused x64/x86 Board smoke passed.  The authorized owner-supplied selected
set probe advanced through the aperture on both widths and next stopped at the
previously unmodeled Board address `0x470` (`1136` decimal):

```text
reason=7 detail=2147483648 fault-code=1136 control-valid=1
control-physical=1136 control-bytes=1 control-access=0
executed=76114 ticks=1205615
```

That next address is not absorbed by this S.  Full cached unit qualification
and the isolated dependency gate remain required before S18 closes.
