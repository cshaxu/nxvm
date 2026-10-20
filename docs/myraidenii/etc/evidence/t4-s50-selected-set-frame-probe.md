# M1 T4 S50 Selected-Set Frame Probe

## Purpose

This is a bounded integration qualification of the already-owned MyRaidenII
Core ingress and frame path. It does not establish a Product, Window, audio,
playable game, physical-hardware or desktop result.

## Method

The existing owner-supplied external `raiden2.zip` was read without retaining
any member in the repository. A transient ignored `build/` directory held the
two V30 program lanes and seven fixed graphics roles while the existing probe
ran. The probe passes the complete role bundle once to Board creation, releases
all caller buffers after Board copies them, executes at most 2,000,000 V30
instructions, and invokes the existing Board frame-copy API once. It prints
only neutral execution and frame statistics; it emits no ROM bytes, pixels,
screenshots or trace.

The roles were supplied in this fixed interface order: program even, program
odd, character image, background image 1, background image 2, then the four
sprite images. Existing Core role-size validation remains the authority for
acceptance.

## Observations

Both cached host widths completed their budget without a terminal CPU, COP or
Board-control fault:

| Host | Executed | Ticks | Frame | Non-black pixels | Frame digest |
| --- | ---: | ---: | --- | ---: | --- |
| x64 | 2,000,000 | 33,301,143 | 320 by 240 | 74,795 | `a06b9c216e1ce034` |
| x86 | 2,000,000 | 33,301,143 | 320 by 240 | 74,795 | `a06b9c216e1ce034` |

The digest is a bounded FNV-style observation of the completed RGB32 frame,
not an adopted golden image and not a redistribution of any protected content.
It only establishes deterministic agreement for this one execution budget.

## Limits

This result does not prove visual fidelity, rotation, host presentation,
inputs, sound, later game progression, title/attract loop, or arcade gameplay.
Those remain owned by the later Machine/Product and gameplay qualification
packets. The external archive was deleted from staging after each run.
