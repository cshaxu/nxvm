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

Focused S9--S12 CECG owner and Model-40 controls pass. The serial current gate
completed 266 current-gate tests; its final log records 433.48 seconds of
aggregate test time. Documentation governance passes for `vm-0-5-0382` and
the deferred direct-ownership verifier reports 213 rows, 153 owner tests,
7 embedded production tests, 1 type foundation, 2 safely separable production,
50 mixed/inherited production entries and 57 exact residual entries.

The repository's `current-gates-gcc` preset was attempted but its MSYS2 GCC
16.2 host compiler exited without diagnostics while compiling unchanged
`src/type.c`; it therefore is not used as source proof. The complete gate above
ran in the existing working GCC tree, and the two S12 controls were rerun after
the subsequent formatting-only commit.

## Coordinator Acceptance

P1 `6dfc3c62` and its formatting-only review repair `47dbaabc` are accepted
after coordinator actual-change review. The review confirms that Model-40
composition declares only the reset selection while VADP owns all mutable port
routing and state; generic EGA has no CECG monochrome registration; the source
contract does not justify `3x8/3x9` routing; two focused controls, the full
current gate and governance evidence cover the admitted behavior. S12 is
selected-device functional progress only. T386, firmware, physical board
behavior, board timing and L3 closure remain open.
