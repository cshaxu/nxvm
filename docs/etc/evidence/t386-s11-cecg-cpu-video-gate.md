# T386 S11: CECG CPU Video-Memory Decode Gate

`M5:T386:S11:CECG-CPU-VIDEO-GATE:OK`

## Primary Contract

The primary source is Compaq, *COMPAQ Enhanced Color Graphics Board / COMPAQ
Color Monitor Technical Reference Guide*, first edition (December 1986),
sections 4.1--4.2. It was consulted transiently under the source policy; no
source text, ROM, firmware, guest media, local path, hash, binary, or third
party code is retained in Git. The source is available for review at the
[Internet Archive record](https://archive.org/details/trg-enhanced-color-graphics-board-1986-12).

| Primary-determined observable | S11 owner and disposition |
| --- | --- |
| Miscellaneous Output is written through `3C2h`; bit 1 controls CPU access to video memory, with `1` disabled and `0` enabled. | The selected CECG personality registers the `3C2h` write route in shared VADP. VADP owns the mutable bit and restores its declared reset value. |
| The documented effect is CPU access, not a claim that system RAM or board storage exists behind the aperture. | The VADP EGA-provider query declines the aperture while the bit is set. Existing core physical-memory routing then decides any fallback route. S11 does not invent board memory availability. |
| Board configuration is selected by the machine. | Private Model-40 composition in `vm` declares the reset bit as enabled CPU access; it contains no mutable register logic. |

This is a source-bounded CPU-decode contract. It is not dynamic I/O-base
selection, a model of the remaining Miscellaneous Output bits, physical monitor
or signal behavior, firmware programming, ISA contention, or board timing.

## Owner Boundary And Proof

The generic VADP owns reusable port state and memory-provider response. The
Model-40 `vm` composition supplies only the selected reset declaration. Generic
EGA does not register the Compaq `3C2h` route and remains unaffected.

- `core-machine-compaq-cecg-s11-smoke` arms the existing EGA aperture, proves
  selected CECG provider ownership, disables it through `3C2h` bit 1, observes
  the ordinary-memory fallback route, restores the declared state on VADP reset,
  and confirms generic-EGA isolation.
- `vm-model40-cecg-s11-smoke` proves the same selected-profile behavior through
  private Model-40 composition and session reset with a project-owned synthetic
  ROM fixture.

## Similar-Issue Sweep And Transfers

The sweep used:

```text
rg -n -i "3c2|miscellaneous.*output|cpu.*video.*memory|video.*memory" src tests CMakeLists.txt docs
rg -n -i "ega_planar_query|device_provider.*query|cecg|ega_personality" src tests CMakeLists.txt docs
```

The selected production hit is the VADP EGA-provider query and its CECG-only
port registration. The direct configuration, reset, private Model-40
composition, generic-EGA path, tests, strict-owner target inventories and debt
record are updated. No other production `3C2h` owner exists.

S11 transfers all other `3C2h` effects, alternate I/O routing, Input Status 0,
monitor blanking/selection, live-video multiplexing, Special Features hardware,
option-ROM/BIOS programming, physical monitor signalling, raster/service
duration, ISA availability/waits, board memory availability and all board/L3
timing to later DeskPro functional, firmware, board and L3 receivers. It makes
no runnable-public-profile, physical-monitor, firmware, board-timing or L3
claim.

## Verification

Focused S9--S11 CECG owner and Model-40 controls pass. The serial current-gate
reports 264/264 tests passed in 85.11 seconds. Documentation governance and
deferred direct-ownership governance pass; the latter reports 211 rows with
151 owner tests.