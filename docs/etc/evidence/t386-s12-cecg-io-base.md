# T386 S12: CECG I/O-Base Selection

`M5:T386:S12:CECG-IO-BASE:OK`

## Primary Contract

The primary source is Compaq, *COMPAQ Enhanced Color Graphics Board / COMPAQ
Color Monitor Technical Reference Guide*, first edition (December 1986),
sections 3.2 and 4.1--4.2. It was consulted transiently under the source
policy; no source text, ROM, firmware, guest media, local path, hash, binary,
or third-party code is retained in Git. The source is available for review at
the [Internet Archive record](https://archive.org/details/trg-enhanced-color-graphics-board-1986-12).

| Primary-determined observable | S12 owner and disposition |
| --- | --- |
| Miscellaneous Output is written through `3C2h`; bit 0 selects the I/O base: `0` for monochrome `3Bxh`, `1` for color `3Dxh`. | Shared VADP owns the mutable selected-base state and restores its declared reset value. |
| The guide lists mode-dependent `3x4/3x5`, `3xAh`, `3xBh`, and `3xCh` routes. | CECG alone receives the corresponding CRTC, Status/Feature-Control and light-pen route registrations. The route-inactive legacy handler returns the core port abstraction's zero nonresponse; this is not a hardware open-bus claim. |
| Reset board selection is machine-specific. | Private Model-40 composition in `vm` declares only the selected color-base reset state. It owns neither dispatch nor mutable port/register behavior. |

The guide does not establish CECG `3x8/3x9` Mode/Color routing. S12 leaves the
existing shared CGA-facing Mode/Color behavior unchanged rather than inferring
an unlisted Compaq mirror.

## Owner Boundary And Proof

VADP is the reusable core hardware owner for CECG port decode and state.
Model-40 composition supplies only the source-backed reset declaration.
Generic EGA receives no `3Bxh` CECG routes and retains the color-base CRTC
behavior already owned by VADP.

- `core-machine-compaq-cecg-s12-smoke` proves both selected routes, inactive
  CRTC/Feature rejection, monochrome Feature and light-pen state, reset
  restoration, and generic-EGA isolation.
- `vm-model40-cecg-s12-smoke` proves the same selected Model-40 composition
  route through the session lifecycle and reset using a project-owned
  synthetic ROM fixture.

## Similar-Issue Sweep And Transfers

The sweep used:

```text
rg -n -i "3c2|3b[45abc]|3d[45abc]|miscellaneous.*output|feature.*control|lightpen" src tests CMakeLists.txt docs
rg -n -i "cecg|ega_personality|configure_cecg|model40" src tests CMakeLists.txt docs
```

It covers CECG registration, VADP state/configuration/reset, Model-40 reset
declaration, generic EGA isolation, owned tests, current-gate registration,
strict-owner inventories, and the CECG debt record.

S12 transfers Input Status 0, SW1 sense, Special Features inputs, vertical
retrace IRQ, all remaining `3C2h` effects, monitor blanking/selection,
live-video multiplexing, option-ROM/BIOS programming, physical monitor
signalling, raster/service duration, ISA availability/waits, board memory
availability, and all board/L3 timing to later DeskPro functional, firmware,
board and L3 receivers. It makes no profile-publication, firmware, physical
monitor, board-timing or L3 claim.

## Verification

Focused S9--S12 CECG owner and Model-40 controls pass. The required serial
current gate, documentation governance, and deferred direct-ownership
verification are recorded with this S's P1 verification result.