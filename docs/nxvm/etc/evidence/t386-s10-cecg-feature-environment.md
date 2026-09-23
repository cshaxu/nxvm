# T386 S10: CECG Feature Control And Environment

`M5:T386:S10:CECG-FEATURE-ENVIRONMENT:OK`

## Primary Contract

The primary source is Compaq, *COMPAQ Enhanced Color Graphics Board / COMPAQ
Color Monitor Technical Reference Guide*, first edition (December 1986),
sections 4.1--4.2. It was consulted transiently under the source policy; no
source text, ROM, firmware, guest media, local path, hash, binary, or third
party code is retained in Git. The source is available for review at the
[Internet Archive record](https://archive.org/details/trg-enhanced-color-graphics-board-1986-12).

| Primary-determined observable | S10 owner and disposition |
| --- | --- |
| Feature Control is write-only at `3xAh`; for the selected color route that is `3DAh`. Bits 0 and 1 drive FCO and FC1. | CECG personality registers the color-route write owner in shared VADP. It retains only the two source-defined bits. |
| Environment is read-only at `7C6h` and reports the FCO/FC1 signal state. A zero bit means the output is driven high and a one bit means low. | VADP reads the two mutable feature bits through Environment while preserving composition-declared non-feature capability bits. |
| Environment also records whether EGA mode is supported. | The declared Environment upper bits remain fixed board capability; a Feature Control write cannot change them. |
| Configuration/reset state is board-selected. | The CECG declaration supplies the reset Environment value. VADP initializes both direct declaration and reset from its low feature bits. |

This is a register-state contract, not a model of an attached Special Features
board, its electrical pins, live video, physical monitors, firmware, or time.

## Owner Boundary And Proof

VADP remains the only mutable owner of CECG ports and state. Model-40 composition
continues to declare its selected Environment reset value (`00h`) in `vm`; it
has no Feature Control logic. Generic EGA does not register the `3DAh` write
owner and its behavior remains unchanged.

- `core-machine-compaq-cecg-s10-smoke` configures a nonzero declared Environment,
  proves direct declaration initialization, color `3DAh` write-to-`7C6h`
  readback, preservation of the EGA-capability bit, reset restoration, and
  generic-EGA isolation.
- `vm-model40-cecg-s10-smoke` proves private Model-40 composition reaches the
  same owner through `3DAh`, then restores its declared `00h` state on session
  reset.

## Similar-Issue Sweep And Transfers

The sweep used:

```text
rg -n -i "3xah|3dah|7c6|feature.*control|environment|cecg" src tests CMakeLists.txt docs
rg -n -i "compaq.*environment|compaq.*feature|ega_personality" src tests CMakeLists.txt
```

The only selected production state/port owner is `src/core/machine/vadp.c`;
Model-40 composition is the sole selected-machine declaration receiver. No
other feature-control route or generic-EGA write owner exists.

S10 transfers `3C2h` miscellaneous-output behavior, alternate I/O routing,
CPU video-memory gating, Input Status 0, live-video multiplexing, Special
Features hardware, monitor output, firmware mode programming, raster/service
duration, ISA availability/waits, and all board/L3 timing to later DeskPro
functional, firmware, board, and L3 receivers. It makes no runnable-public-
profile, physical-monitor, firmware, board-timing, or L3 claim.
## Verification

Focused S10 owner and Model-40 controls pass. The serial current-gate reports 262/262 tests passed in 76.30 seconds. Documentation governance, VM provider-composition, and deferred direct-ownership governance also pass; the latter reports 209 rows with 149 owner tests.

## Coordinator Acceptance

P1 is accepted after actual-change review of the VADP state/port owner, declaration and reset paths, focused controls, generic-EGA isolation, strict-owner inventory, full current-gate and transfers. This accepts S10 only as selected-device functional progress. T386 and every firmware, physical-media, board-timing and L3 receiver remain open.
