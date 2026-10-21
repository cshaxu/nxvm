# M1 T7 S27 — COPX-D2 Boundary Observation

## Scope and evidence grade

This is a bounded L2 reference observation.  It does not add a local COPX
receiver, derive a lookup rule, retain protected output, or claim a game
qualification.  It answers the narrower question left by S5/S8: whether the
available reference exposes enough observable information to define a finite,
independently implemented COPX-D2 Board contract.

## Static reference facts

The inspected reference is the locally pinned MAME Raiden II driver revision
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, file `raiden2.cpp`, SHA-256
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`.

It declares the selected set's `copx-d2.u0313` as a 0x40000-byte `copx`
region.  Its Raiden II main map attaches the established COP register windows
at 0x00400--0x005bf and trigger locations near 0x006fc.  The inspected driver
does not attach that declared COPX region to a mapped read/write handler,
device callback, request register, index computation, result register, reset
operation, or error path.  This is an L2 implementation/layout observation,
not a hardware statement and not copied implementation material.

## Bounded black-box observation

MAME 0.289 was run once with the owner-supplied `raiden2` archive and the two
already-approved external PAL inputs.  A task-owned temporary archive,
configuration and NVRAM directory lived only under
`O:\repos.hobby\nxvm-assets\media-myraidenii\t7-s27-mame`.  The run used a
public Lua write-tap on four named main-CPU ranges and exited after 600 frames.
It retained no guest bytes, pixels, audio, RAM, trace, or ROM payload.

| Observation | Aggregate writes |
| --- | ---: |
| COP control window 0x00400--0x004ff | 17,278 |
| COP command window 0x00500--0x0057f | 27,720 |
| COP result window 0x00580--0x005bf | 0 |
| COP trigger window 0x006fc--0x006ff | 500 |

The counts show that the existing ordinary COP windows are active during this
bounded startup.  They do not identify a COPX byte request, index, response
width/order, lifecycle rule, or a dependency on the separate image.  The
reference's region declaration plus these aggregate writes therefore cannot
form a safe receiver contract.

## Disposition

COPX-D2 remains an externally identified required image with **no admitted
local consumer**.  The correct current implementation remains to keep it out
of the twelve-image transaction rather than add an inert loader or a guessed
lookup device.  A later receiver task needs a finite L2 or L3 contract that
names the request mechanism, address/index calculation, result ordering,
reset/failure behavior and a synthetic non-ROM oracle.

The temporary MAME stage and Lua script were removed after the process exited.
This observation does not qualify title/attract, credits/start, graphics,
sound, gameplay or release.
