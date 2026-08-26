# T484 S11 XT CGA Plan Implementation

`M5:T484:S11:XT-CGA-PLAN:OK`

`M5:T484:S11:XT-NO-VIDEO-ALIAS:OK`

## Result

The selected IBM CGA adapter is now part of the immutable 5160-268 copied
Core plan.  Its only guest-facing route is the established VADP owner:

```text
CGA: B8000h--BFFFFh VADP memory aperture
     3D4h/3D5h CRTC, 3D8h mode, 3D9h color, 3DAh status
```

The plan selects neither EGA nor VGA.  It creates no adapter ROM, BIOS/VM
mode mirror, renderer-owned video memory, generic XT video abstraction, or
session route.  The 5160 profile remains unavailable as a runnable session
until its independent ROM/B6 receiver is implemented.

## One Owner

`core_machine_vadp` remains the sole owner of the CGA register state, mapped
VRAM and copied display snapshot.  The XT profile only supplies the copied
display declaration.  As with existing plans, `48/8/8` is the established
non-physical VADP scheduling contract required to construct that owner; it is
not an IBM 5160 display-clock, monitor, retrace or host-pacing claim.

The retained data flow is:

```text
guest port/memory write -> Core VADP state -> copied Core display snapshot
```

## Focused Proof

The existing `vm-xt-5160-268-profile-smoke` now proves the frozen declaration
selects CGA only, then materializes the copied Core plan and verifies the CGA
port grammar.  It writes one text cell at `B8000h` through the public Core
memory boundary and captures it through the public Core display snapshot;
the resulting `X`/`1Fh` cell proves the aperture and snapshot are the one
VADP route.  It also proves there is no EGA attribute-port provider at `3C0h`.

Existing VADP text, CGA graphics/640-port, and IBM 5170 resolver smokes pass,
showing the selected XT declaration does not change the shared VADP or AT
paths.  The full Debug `current-gate` passed.  The stripped Release target
`vm-0-5-0484` passed and retained
`build/output/nxvm_0_5_0484.exe` with SHA-256
`73D252A9C79D74725898207D43D3DCDEC01D2A1CDA7A527F2591564E5E0E25CA`.
Documentation governance passed.

## Simplicity Accounting And Transfer

Production changes add one copied display declaration to the XT resolver; the
focused existing smoke adds one plan-level route test.  No Core or VADP source
changed, no target was added, and no second state owner exists.

IBM-CGA port and aperture semantics are handled by the accepted VADP route.
Physical crystal/display generation, monitor behavior, light pen and any
hardware display deadline remain deliberately absent and transfer to the XT
board/device phase-timing receiver.
