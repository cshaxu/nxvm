# M5 T254: Digital CGA And 6845 Visible Behavior

**Status:** S1 active.

## Original Request

Extend the core-owned digital CGA device from the retained text and
`320x200x4` paths to one bounded `640x200x2` mode and only the 6845-visible
behavior a compact corpus needs. `core/machine/vadp` is the sole guest-video
state owner; profile supplies frozen defaults and ROM admission, composition
copies snapshots, and platform only renders copied payloads.

## S1 Contract And Corpus

### Current Evidence

| Surface | Current owner and behavior | T254 disposition |
| --- | --- | --- |
| `3D4h/3D5h` | VADP owns one CRTC index and registers `0Ah`--`0Fh`: cursor shape, start address, and cursor address. | Admit only `0Ch`--`0Fh`; do not add timing/geometry registers unless the corpus needs them. |
| `3D8h/3D9h` | VADP owns mode/color. `02h` selects `320x200x4`; a graphics-plus-high-resolution write is currently rejected and retains the prior mode. | Admit `1Ah` as `640x200x2` only. |
| `3DAh` | VADP exposes read-only tick-derived display-enable/retrace status. | Retain unchanged; no read side effect. |
| `B8000h` | Core memory route is the sole CGA text/graphics backing. | `640x200x2` uses the same 16 KiB aperture, 80 bytes/scanline and odd-row bank at `+2000h`; no shadow. |
| Snapshot | Core produces copied text, `CGA_320X200X4`, and EGA payloads. | Add one platform-neutral copied `CGA_640X200X2` indexed-pixel kind. |
| ROM/profile | Default profile owns ROM/INT 10h admission and BDA bookkeeping. | S2 may admit only the defined mode service through ports and memory; no direct VADP/framebuffer mutation. |

### Admitted Slice

- `640x200x2`, selected by mode-control `1Ah` (`graphics`, video-enable,
  high-resolution); all other high-resolution combinations retain the current
  text-snapshot fallback until separately admitted.
- Memory is interleaved CGA digital layout: each even/odd scanline pair uses
  80 bytes, with odd scanlines at `B8000h + 2000h`; byte bits map MSB-first to
  two indexed pixels. Color-select bit 0 selects the foreground RGBI color;
  index 0 is black, index 1 is that color. No composite artifact colors.
- The CRTC subset is only start address (`0Ch/0Dh`) and cursor location/shape
  (`0Ah`--`0Fh`). Start address must affect copied text snapshots; whether it
  affects the narrow graphics slice is deliberately deferred unless the S3
  corpus demonstrates a DOS need.
- Raster status stays derived exclusively from existing elapsed ticks.

### Corpus

1. **S1 baseline port probe:** records the currently deterministic unsupported
   `1Ah` result: `3D8h` retains its prior mode, `3D9h` remains writable, `3DAh`
   remains read-only, and the snapshot remains text. This is a
   pre-implementation negative control.
2. **S2 port/frame probe:** will assert `1Ah` selection, interleaved bank
   addressing, MSB-first bit packing, color-select foreground, text fallback,
   and the admitted CRTC fields.
3. **S3 DOS fixture:** a temporary FDD image will receive an owner-built COM
   program, boot DOS, execute it through normal KBC ingress, select the mode
   through guest ports, write `B8000h`, and prove the copied frame. It will
   retain CGA text/`320x200x4`, EGA, Console/debugger, FDD/HDD boot coverage.

### Deferred

6845 geometry registers, text 40/80-column reconfiguration, graphics page
scrolling/start-address effects, light pen, composite artifact color, EGA/VGA,
DAC, planar/latch, VBE, renderer features, mouse API, BIOS shortcuts, and
cycle-exact video timing are not T254 behavior.

## Ownership And Rules

- `core/machine/vadp` owns registers, VRAM route, dirty state, elapsed-tick
  raster state, and copied snapshot generation.
- `vm/profile/default_profile` may bind frozen defaults and make normal ROM
  port/memory calls. `vm/composition` copies a snapshot. `vm/platform` never
  reads VRAM or mutates VADP.
- Applicable rules: one VADP state owner and one memory route; no core-to-VM
  dependency; no platform guest mutation; no host-clock raster; no raw VRAM
  pointer or second frame/state mirror.

## Similar-Issue Sweep

Compatibility class: every existing CGA-mode decision and snapshot consumer.
S1 inventory is `rg -n "CGA_320X200|CGA.*GRAPHICS|3d[4589a]|B8000|capture.*snapshot"
src tests CMakeLists.txt`. Production hits are VADP, default-profile firmware,
and composition display capture only; no platform VRAM reader exists. S2 must
re-run this query, classify every new high-resolution branch, and add a static
ownership gate if the new display kind introduces a mechanically detectable
forbidden platform/core shape.

## Commands And Exit

- S1: build and run `core-machine-cga-640-port-smoke`; expected marker
  `M5:T254:S1:CGA-640-BASELINE:PORT:OK`.
- S2: focused VADP probe and static ownership/dependency gates.
- S3: `current-gates-gcc`, complete CTest, `current-gcc`, then record artifact
  `nxvm_0_5_0253.exe` SHA-256.

Stop and split if a requirement needs platform VRAM access/mutation, VM-owned
VADP state, host-time raster progression, undefined mode semantics, or a
Console/debugger/boot experience change.
