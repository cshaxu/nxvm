# M5 T236: EGA/VGA Graphics And Attribute Controllers

## S1--S4 Complete

### Objective

Admit the bounded graphics-controller and attribute-controller register
families promised by T235. The task owns controller protocol, reset/readback,
and map-select classification. It does not claim EGA/VGA rendering or planar
data-path correctness.

### Owners And Boundaries

| Concern | Owner | Rule |
| --- | --- | --- |
| Graphics/attribute index, registers, attribute flip-flop, selected aperture, dirty generation | `core/machine/vadp` | One VADP controller state; no video shadow or alternate device. |
| Guest byte storage | `core/machine/memory` | One RAM backing; VADP observes configured video-window writes only. |
| Default ports and reset values | `vm/profile/default_profile` | Immutable descriptor, bound once during the existing configuration window. |
| Binding and freeze | `vm/composition` | Calls the core profile binding once; no VM-side display loop. |
| Rendering and product UI | composition/platform | Consume copied frames only. T236 introduces no frame kind or platform change. |

No IRQ, DMA, firmware/BDA shortcut, host clock, host renderer, second RAM
backing, or direct platform mutation is admitted.

### Admitted S2 Register Slice

- Graphics-controller index/data ports `3CEh`/`3CFh`, registers 0--8. Each
  stores a documented masked value and reads back through the selected index.
  Unsupported indexes read `FFh` and ignore data writes.
- Graphics register 6 (miscellaneous graphics) selects one VADP active video
  aperture: A0000h--BFFFFh (128 KiB), A0000h--AFFFFh (64 KiB),
  B0000h--B7FFFh (32 KiB), or B8000h--BFFFFh (32 KiB). It changes VADP window
  classification and dirty observation only.
- Attribute-controller address/data port `3C0h`, read port `3C1h`, and the
  input-status-1 read at `3DAh`. A `3DAh` read preserves raster bits but resets
  the attribute index/data flip-flop. `3C0h` alternates index then data;
  `3C1h` reads the selected supported register.
- Attribute indexes 00h--14h store masked controller state. Palette/DAC
  meaning, color selection, panning, and rendering are not claimed yet.

The existing sequencer map mask and graphics registers are observable control
state only. A map-select write does not hide ordinary system RAM nor transform
plane bytes in T236: doing so needs T238's sole planar/latch data path. The
focused planar-access fixture will prove this explicitly rather than infer a
fake implementation from raw RAM reads.

### Reset And Unsupported Behavior

The default profile supplies graphics-register reset values, including
miscellaneous-graphics `05h` (A0000h 64 KiB), and attribute reset values. The
attribute flip-flop begins in index phase. Disabled/unconfigured controller
ports read `FFh` and ignore writes. Unsupported graphics/attribute index data
reads return `FFh`; writes leave all supported state unchanged.

### S1 Probe Plan

Add `tests/machine/core_machine_ega_controller_port_smoke.c` and
`core-machine-ega-controller-port-smoke`, marker
`M5:T236:S1:EGA-CONTROLLER:PORT:OK`. It must cover:

1. cold reset, profile defaults, masks, selected-index readback, and unsupported
   index non-aliasing for `3CEh`/`3CFh`;
2. all four graphics map-select ranges, boundary classification, and dirty
   generation for the selected range only;
3. `3C0h` index/data alternation, `3C1h` readback, status-read flip-flop reset,
   display-enable bit retention, and unsupported attribute-index behavior; and
4. a planar-access negative fixture: sequencer map-mask plus graphics control
   registers remain stored state and direct RAM bytes remain untransformed.

S3 adds `vm_ega_controller_system_smoke.c` against an ordinary frozen session.
No DOS EGA/VGA fixture is applicable until T238 creates a copied frame path.

### Required Matrix And Stop Conditions

Focused port/memory probe, profile/system probe, static ownership gates, and
the full current GCC/CTest matrix are required. Retain CGA text/graphics, DOS
prompt, Console/debugger, FDD/HDD boot, and session isolation coverage.

Stop and return for design approval if implementation requires a second video
object, host renderer VRAM access, a firmware or BDA shortcut, a VM-side video
loop, direct platform guest mutation, raw unbounded trace, or a claim of
planar/latch/DAC/raster support.

The similar-issue sweep covers all graphics/attribute port ownership, video
memory-map declarations, and any existing direct video-memory routing.

## Completion Record

### S2 Implementation

VADP now owns graphics-controller registers 00h--08h behind `3CEh`/`3CFh`
and attribute-controller registers 00h--14h behind `3C0h`/`3C1h`. Register
write masks, selected-index readback, reset values, and unsupported-index
`FFh` behavior are owner-local. A `3DAh` read continues to report its existing
raster status and additionally resets only the attribute index/data phase.

Graphics register 06h selects the active VADP aperture classification. The
existing RAM write observer advances the single VADP dirty generation only for
the selected range. RAM still stores every byte, and no map select creates an
overlay, hides ordinary RAM, transforms planes, or produces a frame.

### S3 Evidence And Matrix

| Surface | Evidence |
| --- | --- |
| Focused port/memory contract | `core-machine-ega-controller-port-smoke` passed with `M5:T236:S1:EGA-CONTROLLER:PORT:OK`. It covers graphics reset/masks/unsupported index, all four aperture classifications, selected-window dirty routing, attribute index/data/flip-flop/reset behavior, and the planar negative fixture. |
| Profile/composition integration | `vm-ega-controller-system-smoke` passed with `M5:T236:S3:EGA-CONTROLLER:SYSTEM:OK`. It uses an ordinary frozen VM session to verify profile-reset controller ports, attribute protocol, and B0000h backing. |
| Owner/dependency shape | `verify-ega-controller-boundary` passed with `M5:T236:EGA-CONTROLLER:BOUNDARY:OK`; it rejects VM/platform imports from VADP and controller policy in RAM. |
| Retained machine/product behavior | GCC 16.1.0 `current-gates-gcc` passed 71/71 current CTest smokes, including CGA text/graphics, DOS prompt/keyboard/video, Console/debugger, session isolation, and FDD/HDD boot. |
| DOS EGA/VGA fixture | Not applicable. T236 has no EGA/VGA raster, copied snapshot, or BIOS mode. T238 must add the first guest-visible EGA/VGA fixture. |

Commands: `cmake --build --preset current-gates-gcc` and
`cmake --build --preset current-gcc`. The current runnable artifact is
`build/output/nxvm_0_5_0236.exe`, SHA-256
`738F17139BEB898B75EE9C397F49CE6163DE419252D867F995082CB8B98589CE`.

### Similar-Issue Sweep

The corrected class is controller state or video-window routing implemented
outside VADP. The closure query was
`rg -n '0x0*3c[01ef]u|GRAPHICS_REGISTER|ATTRIBUTE_REGISTER|ega_controller|core_machine_vadp.*(graphics|attribute)' src tests CMakeLists.txt cmake docs TODO.md`.
Production hits are the VADP owner, one immutable profile declaration, and one
composition binding; tests and the closure gate are the remaining hits. The
sole unrelated match is `cpu_int_ivt_smoke` writing the byte address `03C0h`,
which is IVT test data rather than a port or video route. No production hit was
deferred.

### Remaining Deferrals

T238 alone may implement planar/latch access, DAC, raster state, copied EGA/VGA
snapshots, and a guest-visible DOS graphics fixture. T236 makes no claim that
an EGA/VGA program can yet display output correctly.

### S4 Audit Closure

S4 normalizes every profile-supplied graphics and attribute reset value through
the same per-register masks used by port writes. The focused controller probe
now supplies deliberately invalid reset bytes and proves that the reset
readback equals the port-admissible value. The RAM-closure gate now uses an
unambiguous prefix check, so it emits no CMake developer warning.

The full GCC/CTest matrix and focused probes pass after S4. This is a T236
source correction, so the task artifact is rebuilt under its existing task
identity; its refreshed SHA-256 is
`738F17139BEB898B75EE9C397F49CE6163DE419252D867F995082CB8B98589CE`.
