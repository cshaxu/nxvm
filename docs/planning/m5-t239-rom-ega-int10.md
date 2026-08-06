# M5 T239: Default-ROM EGA INT 10h Mode Service

## S1: Contract And Fixture Design

### Objective

T239 admits one default-PC/AT ROM video mode service for the already admitted
`EGA-320x200x16-direct` VADP surface.  `INT 10h`, `AH=00h`, `AL=0Dh` selects
that mode; `AH=00h`, `AL=03h` selects the retained 80-column text mode and
therefore exits it.  No other EGA/VGA mode, DAC, VBE, renderer path, or host
shortcut is admitted.

The ROM is executable 8086 firmware.  It must establish both modes through
ordinary guest `OUT` instructions and guest memory writes:

| Concern | Sole owner | T239 rule |
| --- | --- | --- |
| Register, planar VRAM, latches, snapshot | `core/machine/vadp` | VADP remains the only video state owner and receives only its existing port and memory transactions. |
| Mode service and BDA cache | default-profile ROM | The ROM decodes `INT 10h` and writes port registers plus only the BDA fields it serves. |
| Mode selection topology | `vm/profile/default_profile` | The profile keeps its frozen VADP capability and ROM service declaration. |
| Frame copy and presentation | VM composition/platform | Existing copied-snapshot policy is unchanged; neither layer sees guest VRAM. |

`AL=0Dh` writes the already admitted sequencer map-mask, graphics mode/map,
and attribute graphics-enable subset.  In particular, the graphics-controller
map select is written to the A0000h 64 KiB value, which arms the existing VADP
planar path.  `AL=03h` writes the graphics-controller map select back to a
non-A0000h value before the existing text path continues, so leaving the mode
also travels through the same real VADP port state.

The only new BIOS data area truth is `40:49` (current video mode), because the
existing `AH=0Fh` service reads it.  It is a ROM-owned query cache, not a VADP
mirror: VADP register and memory state remain authoritative for rendering.
Existing text fields and all existing text `INT 10h` functions retain their
current behavior.  Unsupported `AH=00h` mode values remain unsupported under
the existing return semantics.

### Validation

S2/S3 evolve the following focused evidence without adding a second video
route:

- `vm-rom-ega-int10-system-smoke` boots an 8086 fixture, invokes `INT 10h`
  mode `0Dh`, writes A0000h, observes the copied EGA frame and BDA `40:49`,
  then invokes mode `03h` and observes text plus the restored BDA value.
- `vm-rom-ega-int10-dos-smoke` temporarily copies the owner-provided DOS
  image, installs a small `.COM` program that invokes mode `0Dh` and writes
  A0000h, boots through the retained KBC/ROM/DOS path, and observes the copied
  EGA frame.  Guest media stays untracked.
- `verify-rom-ega-int10-boundary` rejects direct VADP, VRAM, display, or
  platform access from the ROM helper and requires the firmware port sequence.
- The existing T238 direct-port probe remains in the current matrix, followed
  by CGA text/graphics, Console/debugger, FDD/HDD boot, and `current-gates-gcc`.

### Stop Conditions

Stop for a contract amendment if this requires direct VADP mutation, a VRAM
backing pointer, BDA-to-VADP synchronization, a renderer change, a second
machine/session, a host-clock path, or a mode outside `03h` and `0Dh`.

S1 changes no guest behavior and produces no task artifact.

## S2 Implementation And S3 Closure

The default-ROM `INT 10h` instruction stream now recognizes `AH=00h` with only
`AL=0Dh` and `AL=03h` as admitted mode-set values.  `0Dh` writes the existing
sequencer map mask, graphics mode/map select, and attribute graphics-enable
ports, then writes BDA `40:49 = 0Dh`.  The existing VADP port owner receives
those guest `OUT` operations and activates the retained T238 A0000h planar
provider.  `03h` first writes the graphics map select to `09h`, which disarms
that same VADP path, then records BDA `40:49 = 03h`.  No firmware code imports
or calls VADP, accesses planar backing, or changes composition/platform state.

Existing `AH=02h`, `06h`, `0Bh`, `0Eh`, and `0Fh` text behavior is retained.
`AH=0Fh` now observes the same ROM-owned BDA mode cache for both admitted
values; it does not inspect or synthesize VADP state.  Unsupported mode values
retain the prior no-op return behavior.

| Evidence | Result |
| --- | --- |
| Boundary gate | `verify-rom-ega-int10-boundary` passed with `M5:T239:ROM-EGA-INT10:BOUNDARY:OK`; it rejects VADP/platform shortcuts from the ROM source and requires the port-driven `0Dh`/`03h` sequence. |
| Retained core probe | `core-machine-ega-planar-port-smoke` remains in the current matrix and passed, preserving the T238 direct-port contract. |
| 8086 system fixture | `vm-rom-ega-int10-system-smoke` passed with `M5:T239:S3:ROM-EGA-INT10:SYSTEM:OK`; generated boot code invokes `0Dh`, queries `AH=0Fh`, writes A0000h, observes the copied EGA frame, invokes `03h`, and observes text plus both BDA/query values. |
| DOS fixture | `vm-rom-ega-int10-dos-smoke ../fdd.img` passed with `M5:T239:S3:ROM-EGA-INT10:DOS:OK`; it copies the owner-provided DOS image, installs `EGAT239.COM`, types the command through KBC/ROM/DOS, and observes the copied EGA frame. |
| Retained behavior | GCC 16.1.0 `current-gates-gcc` passed its static/boundary gates and 76/76 current CTest smokes, including CGA text/graphics, direct EGA, DOS prompt, `EDIT.COM`, Console/debugger, session isolation, FDD boot, and HDD boot. |
| Artifact | `build/output/nxvm_0_5_0239.exe`, SHA-256 `4C4FDC4DCEB1CE0003A71E54C3DFFA19101AC048E8E7B62919596E6E126123DA`, built by `current-gcc`. |

No DAC/VBE, other EGA/VGA family, host renderer shortcut, Console/debugger,
or startup/boot behavior was changed.  This closes T239.
