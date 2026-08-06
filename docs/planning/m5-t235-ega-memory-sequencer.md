# M5 T235: EGA/VGA Memory Window And Sequencer

## S1--S3 Complete

### Objective

Define the first bounded EGA/VGA controller admission: sequencer register
ownership and a profile-selected EGA/VGA memory aperture. This subtask does
not implement guest-visible EGA/VGA behavior. It defines the S2 owner, port
and memory probe, deferred register families, and stop conditions before any
device source changes.

### Baseline

T228 provides the only admitted video controller behavior: CGA text and
`320x200x4` through core-owned VADP ports `3D4h`--`3DAh` and B8000h-backed
memory. T234 closed the peer-dependency and opaque-session boundaries. The
current artifact is `0.5.0234`; T235 allocates the next runnable revision only
when an S2/S3 behavior change has passed its evidence.

### Ownership And Boundaries

| Concern | Owner | Rule |
| --- | --- | --- |
| Sequencer register state, EGA/VGA aperture state, dirty generation, and later scanout | `core/machine/vadp` | One VADP object is the only guest-video controller-state owner. |
| Guest RAM bytes, including the admitted aperture backing | `core/machine/memory` | One RAM backing stores bytes. VADP observes configured aperture writes; it never creates a shadow buffer. |
| Default adapter topology, aperture admission, reset register values, and ROM assets | `vm/profile/default_profile` | Frozen description only; it does not map memory or mutate registers. |
| Provider binding and profile-to-core configuration | `vm/composition` | Performed only during the core configuration window, then frozen. |
| Frames, renderers, Console/window/auto selection | composition plus platform | Consume copied snapshots only; never guest VRAM or VADP internals. |

`core/machine` remains independent of VM, profile, firmware, platform, and
product code. No IRQ, DMA request, BIOS interrupt, BDA write, host clock, or
host rendering operation belongs to T235.

### Admitted S2 Slice

S2 may admit only these controller surfaces:

- sequencer index/data ports `3C4h` and `3C5h`, with a declared reset state,
  supported-index mask, index/data readback, and deterministic unsupported
  index behavior;
- a profile-selected A0000h aperture with a declared byte range and checked
  bus-memory behavior. VADP owns its mapping semantics while core RAM retains
  the only byte backing; and
- a single VADP dirty generation shared with current CGA state.

The initial sequencer subset is register state only: reset, clocking mode, map
mask, and memory mode. Its values are stored and probed but do not yet claim
planar write behavior, character-map fetch, display timing, or presentation.
The aperture is not a second frame buffer and must not create a platform- or
firmware-owned shadow copy. Until T236 admits graphics-controller map-select,
addresses outside the configured aperture retain their ordinary RAM semantics;
they do not alias the aperture.

### Explicit Deferrals

- `3CEh`/`3CFh` graphics-controller registers, including memory-map select,
  read/write modes, set/reset, rotate, bit mask, and logical operations,
  belong to T236.
- Attribute controller, input-status flip-flop, and palette behavior belong to
  T236.
- DAC, planar/latch access semantics, raster scanout, EGA/VGA text rendering,
  and copied EGA/VGA frames belong to T238.
- EGA/VGA BIOS INT 10h modes, external ROMs, VBE, IRQ/DMA behavior, and host
  presentation changes are out of scope.
- Existing CGA B8000h text and `320x200x4` behavior remain unchanged. B0000h
  and B8000h EGA/VGA map selection are deferred until T236 owns the graphics
  controller map-select rule.

### S1 Probe Plan

Before S2 changes device behavior, add
`tests/machine/core_machine_ega_sequencer_port_smoke.c` and the target
`core-machine-ega-sequencer-port-smoke`. Its success marker is
`M5:T235:S1:EGA-SEQUENCER:PORT:OK`.

The probe will establish the exact S2 acceptance sequence:

1. cold reset gives the documented sequencer index and supported-register
   values;
2. writes and reads through `3C4h`/`3C5h` affect only the selected register;
3. unsupported indexes and out-of-range aperture accesses have defined,
   non-aliasing failures;
4. A0000h writes and reads use the same VADP owner and advance its dirty
   generation without affecting CGA B8000h state; and
5. no IRQ, DMA, firmware call, host event, or platform frame is required.

S1 is complete when this probe passes with the documented reset and negative
cases. It is updated for every admitted S2 branch. S3 adds
`vm_ega_sequencer_system_smoke.c`, which creates the ordinary frozen VM session
and verifies the profile-bound ports and A0000h backing. A DOS fixture is
explicitly not applicable until T238 produces a guest-visible EGA/VGA frame
path.

### Required Regression Matrix

| Surface | S1 disposition |
| --- | --- |
| Focused port/memory probe | New T235 probe, required before S2. |
| Owner/dependency shape | Current DAG, core-utils, session-layout gates. |
| FDD/HDD boot and DOS prompt | Deferred to S2/S3 because S1 changes no runtime behavior. |
| Console/debugger/session isolation | Deferred to S2/S3; no product path changes in S1. |
| Existing CGA graphics/text | Preserve through current VADP/CGA smokes. |

### S1 Stop Conditions

Stop before S2 and return for a new design decision if any proposed change
requires a second video-state object, a raw host renderer VRAM borrow, a
firmware/BDA shortcut, a VM-side video execution loop, a direct platform
guest-state mutation, or an unbounded "VGA support" claim.

Applicable rules: the M5 hardware-device template; core/VADP single-owner
rules; profile immutability; composition-only assembly; no host-clock guest
shortcut; no second session, executor, or machine; retained NXVM
Console/debugger/boot behavior; and the T235-focused similar-issue sweep for
all existing video port/memory mappings.

## Completion Record

### S2 Implementation

`core/machine/vadp` now owns one profile-configured sequencer state with index
and data ports `3C4h`/`3C5h`. The admitted register slice is index 0 (reset),
1 (clocking mode), 2 (map mask), and 4 (memory mode); index 3 and every other
index read as `FFh` and ignore data writes. Masked readback is deterministic.

The default PC/AT profile declares `3C4h`--`3C5h`, A0000h--AFFFFh, and reset
values. During the existing INITIALIZED-only profile binding, composition
registers the VADP with the core RAM write-observer list. RAM remains the only
byte storage. A successful write overlapping the admitted aperture advances
the one VADP dirty generation; it never creates a new buffer, maps a renderer,
or changes CGA behavior.

### S3 Evidence And Matrix

| Surface | Evidence |
| --- | --- |
| Focused contract | `core-machine-ega-sequencer-port-smoke` passed with `M5:T235:S1:EGA-SEQUENCER:PORT:OK`. It checks reset, masks, unsupported index, A0000h read/write, dirty generation, B8000h isolation, and aperture boundary. |
| Profile/composition integration | `vm-ega-sequencer-system-smoke` passed with `M5:T235:S3:EGA-SEQUENCER:SYSTEM:OK`. It creates the ordinary frozen VM session and checks the bound port and backing path. |
| Owner/dependency shape | `verify-ega-sequencer-boundary`, dependency DAG, live-machine, facade, executor, session-readiness, and C-facade gates passed. |
| FDD/HDD, DOS prompt, Console/debugger, isolation | Covered by `current-gates-gcc`: 69/69 current CTest smokes passed, including FDD/HDD boot, DOS prompt, keyboard, CGA graphics DOS, Console lifecycle, debugger, and two-session tests. |
| DOS EGA/VGA fixture | Not applicable: T235 has no EGA/VGA scanout or BIOS mode. T238 must add the first guest-visible graphics fixture after copied frames exist. |

Commands: `cmake --build --preset current-gates-gcc` and
`cmake --build --preset current-gcc`, using the configured GCC 16.1.0 toolchain.
The current runnable artifact is `build/output/nxvm_0_5_0235.exe`, SHA-256
`6A74104435FB12321B142697AFA02628E2B2DDA03073CF323125A3BC8C429B52`.

### Similar-Issue Sweep

The defect class was a potential second EGA aperture/VRAM state or a
profile/platform shortcut. The closure query was
`rg -n '0x0*3c[45]u|0x0*a0000u|EGA_APERTURE|ega_sequencer|write_observer' src tests CMakeLists.txt cmake docs TODO.md`.
Every production hit is classified: VADP owns controller state, RAM owns the
only backing plus generic observer dispatch, the default profile declares
topology, and VM composition binds once. The boundary gate rejects core VADP
product/platform imports and RAM video policy. No production hit was deferred.

### Remaining Deferrals

T236 owns graphics-controller map select and attribute-controller behavior.
T238 owns the bounded planar/latch access, raster scanout, copied fixed-RGBI
frame, and first EGA DOS fixture. Programmable DAC behavior remains deferred.
This task makes no EGA/VGA presentation or compatibility claim beyond the
admitted sequencer/memory-window surface.
