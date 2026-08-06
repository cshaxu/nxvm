# M5 T238: Bounded EGA/VGA Planar Frame Data Path

## S1: Contract And Focused Probe

### Objective

T238 admits one guest-visible planar graphics path without changing the NXVM
Console, debugger, boot flow, CGA text, or CGA `320x200x4` behavior. The sole
candidate is `EGA-320x200x16-direct`: a direct-port, EGA-compatible 320 by 200
16-colour planar surface. It is not a BIOS mode claim; ROM `INT 10h` mode
selection belongs to T239.

S1 changes no guest behavior and produces no artifact. It fixes the S2/S3
contract, adds the evolving core port-contract probe, and requires the current
GCC/CTest baseline before S2 begins.

### Owners And Storage

| Concern | Sole owner | Contract |
| --- | --- | --- |
| Physical address routing | `core/machine/memory` | During `INITIALIZED`, core registers one frozen device-memory provider for an exact physical range. Every CPU, debug, firmware, and core physical read/write uses that same router. |
| Planar VRAM, latches, controller/raster state | `core/machine/vadp` | VADP owns four planes, latches, dirty state, and scanout. Device VRAM is never a RAM mirror and is never exposed as a raw pointer. |
| Port/reset/topology declaration | `vm/profile/default_profile` | The immutable profile declares only reset values and the admitted VADP capability; composition binds it once, then freezes it. |
| Frame copying and display policy | VM composition/platform | Composition copies a VADP snapshot into a platform frame. Platform consumes that copy only; Console rejects graphics clearly, while existing window/auto policy consumes the same frame. |
| BIOS mode service | Default ROM, T239 | T238 adds none. The fixture programs guest ports and video memory directly. |

The existing RAM allocation remains the sole ordinary-system-RAM backing.
T238 S2 may add one VADP-owned device-VRAM allocation, mapped only through the
frozen core memory-provider route. This is real device storage, not a second
machine, RAM alias, synchronised shadow, or platform-visible framebuffer.

### Admitted Surface

`EGA-320x200x16-direct` uses the existing `A0000h--AFFFFh` 64 KiB selected
aperture. Its raster consumes the first 8 KiB of each of four VADP-owned planes:
one byte addresses eight horizontal pixels, bit 7 is the leftmost pixel, and
the four plane bits form a 4-bit indexed pixel. The remaining aperture offsets
are device-addressable but have no scanout meaning in this task; they must not
alias raster bytes.

The mode is active only when the S2 fixture's direct port sequence establishes
the documented profile capability, A0000h 64 KiB map select, graphics mode 0,
and attribute graphics-enable state. S1 does not claim compatibility with an
arbitrary program's register sequence.

S2 may make only these existing registers functional for this mode:

- sequencer index 2 map mask;
- graphics indexes 0/1 set-reset and enable-set-reset, 3 data-rotate/logical
  operation, 4 read-map select, 5 read/write mode, 6 memory-map select, and 8
  bit mask; and
- attribute indexes 0--15 palette mapping, 10 mode control, and 12 colour-plane
  enable.

The first implementation supports read mode 0 and write mode 0 only. It loads
latches on a device-memory read; write mode 0 applies rotate, optional
set-reset, logical operation, map mask, and bit mask to the selected planes.
Read mode 1, write modes 1--3, chain-4, odd/even, text fetch, panning, DAC
ports, 256-colour modes, and all unlisted register effects remain unsupported
and must produce the documented inert/readback behavior without a frame.

### Snapshot And Palette Contract

S2 extends the shared copied indexed-frame contracts from four to sixteen RGB
entries. `EGA-320x200x16-direct` publishes 320 by 200 4-bit pixels and a
16-entry fixed RGBI palette after attribute palette mapping and colour-plane
enable. T238 does not implement VGA DAC ports or programmable DAC state.

The snapshot and platform frame remain fixed-size copied payloads. A changed
VADP plane/controller state increments the VADP dirty generation; composition
increments the session frame generation only when it publishes a changed copied
frame. No renderer, platform, firmware, or profile code reads VADP storage.

### Memory-Provider Contract

T238 S2 introduces a core-private `core_machine_memory_device_provider`
registration with checked byte/block read and write callbacks plus an opaque
owner. Core memory dispatches a matching frozen device provider before ordinary
RAM mapping; otherwise it retains current RAM behavior. Providers are ordered
by non-overlapping physical range, cannot be rebound after freeze, cannot
return a backing pointer, and cannot call back into mutable registration.

VADP receives its own provider callbacks only through its existing
configuration binding. The callbacks operate on VADP-owned planes and latches;
they never copy to ordinary RAM. Out-of-window accesses keep current RAM
semantics. An unsupported EGA configuration leaves the provider inactive and
must not synthesize an indexed frame.

### Focused Probe And Matrix

`core-machine-ega-planar-port-smoke` is the S1 owner-local contract probe. In
S1 it locks the pre-implementation control baseline: profile-style reset,
selected sequencer/graphics/attribute readback, A0000h aperture boundaries,
and the absence of synthetic planar rendering. S2 evolves the same probe with
plane/latch transaction assertions rather than introducing a parallel test
path. Its success marker is `M5:T238:S1:EGA-PLANAR:PORT:OK`.

The similar-issue sweep is:

```text
rg -n 'A0000|B0000|B8000|3C[0-9A-F]|3CE|3CF|sequencer|graphics|attribute|latch|plane|palette|snapshot|write_observer' src tests CMakeLists.txt cmake docs TODO.md
```

S1 classifies production hits as existing VADP owner, core-memory route,
profile binding, composition copy, platform renderer, historical reference, or
explicit deferral. S2 must record the disposition of every new production hit.

Run the focused probe, current static gates, and `current-gates-gcc`. The
retained matrix covers CGA text/graphics, DOS prompt, `EDIT.COM`, Console,
debugger, session isolation, and FDD/HDD boot. S3 adds the direct-port guest
fixture and the `0.5.0238` artifact only after the S2 path exists.

### Stop Conditions

Stop for a design amendment if this surface needs a RAM mirror, a second VADP,
raw host/renderer VRAM access, a BIOS/BDA shortcut, a VM-side video loop,
multiple mode families, a programmable/full DAC, or a change to NXVM Console,
debugger, startup, or boot behavior. Local DOS media and generated probes stay
untracked; only their identity and results may enter evidence.

### S1 Evidence And Closure

The focused `core-machine-ega-planar-port-smoke` passed with
`M5:T238:S1:EGA-PLANAR:PORT:OK`. It locks the existing profile-style reset
values, sequencer/graphics/attribute readback, A0000h 64 KiB aperture
classification, and the current ordinary-RAM access behavior that S2 must
replace only through the frozen device-memory route.

The similar-issue sweep identified existing VADP control ownership, RAM's
write-observer-only route, profile binding, composition copying, and platform
rendering. No pre-existing VADP-owned planar storage, raw renderer VRAM access,
or second guest-memory route was found. The source implementation therefore
remains deliberately unchanged in S1.

GCC 16.1.0 `current-gates-gcc` passed its 32 static/boundary gates and the
complete CTest matrix passed 72/72, including the new probe, retained CGA text
and graphics, DOS prompt/keyboard, `EDIT.COM`, Console/debugger, session
isolation, and FDD/HDD boot. No task artifact is valid for S1: `0.5.0238`
remains reserved for T238 S3 only.
