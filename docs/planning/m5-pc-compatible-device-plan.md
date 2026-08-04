# M5 PC-Compatible Device Plan

## Purpose

This plan records bounded compatibility slices for three retained core devices:
8254 PIT read-back, the Intel 8042-compatible keyboard-controller path, and
the video-adapter state used by the default VM profile. Each design activity
is the `S1` subtask of its corresponding behavior task; no design-only task
number is created. It does not add a DOS runner, change the NXVM Console
grammar, or claim CGA/EGA/VGA compatibility before an owned probe demonstrates
it.

## Current Facts

- `core/machine/pit.*` owns the one PIT state and the completed T191 8254
  read-back slice. Per-mode waveform/GATE/BCD/count-zero compatibility remains
  explicitly deferred; read-back is not a claim of a cycle-accurate PIT.
- `core/machine/kbc.*` owns the completed T192 controller subset: `0x60`/
  `0x64`, OBF/IBF, command byte, bounded byte FIFO, IRQ1, A20/reset, and the
  approved keyboard-command responses. Host input follows the one normalized
  platform-event -> default-profile mapper -> KBC -> QDKEYB INT 09h -> BDA ->
  INT 16h route. Break/extended bytes, scan-set switching, translation, AUX,
  IRQ12, and timing remain deferred.
- `core/machine/vadp.*` owns T193's CGA text slice: CRTC text state, text-mode
  and color registers, stable no-raster status, B8000 visible-window capture,
  dirty generation, and copied text snapshots. Default-profile QDCGA owns INT
  10h/BDA policy only. CGA graphics remains unsupported and separately
  admitted.
- `core_machine` remains the sole owner of PIT/KBC/VADP storage and lifecycle.
  VM composition may bind profile providers and host transports, but may not
  create a second device, reset it directly, or retain mutable aliases.

## Shared Constraints

- Each task uses primary device documentation for behavior, then writes small,
  owned port or snapshot probes. Bochs 2.6 compatibility sources may be read
  only to compare state-machine decomposition and observable behavior: no
  source, global device manager, GUI callback path, C++ plugin structure, or
  build/runtime dependency is copied, transliterated, or linked into the
  product.
- `core` stays independent of `vm` and `vdm`. A core device exposes a frozen
  provider/callback contract where it needs profile behavior; profile firmware
  supplies the behavior through VM composition.
- No task changes retained NXVM Console commands, debugger entry, boot-media
  policy, or host window/Console ownership. FDD/HDD boot and the DOS prompt
  remain regressions for every runnable-path task.
- KBC host input has one route. During and after migration, an input event may
  reach either the old QDKEYB route or the KBC route, never both.
- VADP capability is admitted mode by mode. A text snapshot is not evidence of
  graphics-register, video-memory-window, palette, raster, or timing support.
- A task stops and returns to design if it needs a second guest-state owner,
  core-to-VM dependency, undocumented ROM behavior, an external binary, or a
  user-visible NXVM behavior change.

## Reference Model Boundary

Bochs' keyboard device is a useful model for the separation of controller
command state, IBF/OBF state, keyboard/AUX byte queues, IRQ requests, and
keyboard-device responses. T192 adopts only the keyboard/controller subset as
a core KBC state machine. It does not adopt Bochs' combined global-device,
GUI, or plugin model; AUX and IRQ12 remain a separately admitted task. Its
explicit route is:

```text
platform host event
  -> vm/default-profile keyboard mapper
  -> core KBC controller and byte queues
  -> QDKEYB BIOS INT 09h
  -> BIOS data-area keyboard buffer
  -> BIOS INT 16h
```

Bochs' `vgacore`/`vga` separation likewise informs, but does not define, the
VADP boundary. `core/machine/vadp` owns register state, guest video-memory
mapping, dirty state, and copied scanout snapshots. VM composition reads one
immutable/copy snapshot and converts it to a platform frame. Core/VM platform
code owns renderer/window/Console behavior and never accesses guest VRAM.
QDCGA is BIOS INT 10h firmware, not the video controller. T193 begins with
the CGA text subset only; it does not imply VGA register or graphics support.

## Task Breakdown

### T191: 8254 PIT Read-Back

#### S1: Read-Back Contract And Probe Matrix

**Objective:** freeze 8254 read-back selection, count/status latch order,
repeat-command behavior, observable output/status bits, primary references,
and focused port vectors.

**Approved S1 contract:**

- `0x43` is a read-back command only when `SC1:SC0` is `11`. `D5 == 0`
  requests a count latch; `D4 == 0` requests a status latch. `D3:D1` select
  counters active-low, so every zero bit selects its counter; no selected
  counter is a no-op.
- A selected count is captured only when that counter has no pending count
  latch. A selected status is captured only when it has no pending status
  latch. A later read-back command never overwrites an unread captured value.
- A data-port read returns one pending status byte before any pending count
  bytes. A count latch then follows the programmed RW order (LSB, MSB, or
  LSB-then-MSB), and holds the value captured at command time.
- The status byte is a captured `OUT`, null-count, RW, mode, and BCD snapshot.
  For this task `OUT` reflects the retained counter model: programming mode 0
  drives it low, nonzero modes initialize it high, and a mode-0 terminal count
  drives it high. Full per-mode output waveform accuracy remains separate PIT
  compatibility work.
- Null-count is set after a control word invalidates the active count and clears
  when the current model loads the full initial count. Read-back itself never
  changes count, mode, output callback, or counter readiness.

**Reference and probes:** Intel's 8254 data sheet, *Read-Back Command* and
*Status Read-Back Command*, is the behavioral source. The owned smoke must
cover active-low multi-counter selection; status-before-count order; count-only
and status-only commands; unread-latch preservation; RW byte order; null-count
before/after loading; and unchanged counter operation after read-back.

**Non-goals:** implementation, a generated executable, or a timing-model
rewrite. Bochs is neither a primary specification nor a PIT implementation
reference for this task.

**Exit:** an implementable `S2` contract with exact `pit.*` source surface,
probe cases, retained regressions, and stop condition. No executable is
produced by S1.

#### S2: Read-Back Implementation

**Objective:** implement the bounded 8254 read-back command in the existing
core PIT, without replacing its timer model.

**Owned surface:** `src/core/machine/pit.*` and a focused core PIT port smoke.
No VM/profile source may change except CMake/test wiring.

**Acceptance:** probes cover individual and multi-counter selection, status
versus count latches, read ordering, an already-latched value, and normal
counter operation after a read-back command. Existing timer, FDD/HDD boot,
and DOS-prompt observations remain valid.

**Artifact:** `build/output/nxvm_0_5_0191.exe` after GCC build and retained
runnable regressions.

**Stop:** any evidence that timer behavior requires a broader timing-model
rewrite or unbounded cycle-accuracy work.

### T192: 8042 KBC Compatibility

#### S1: Controller And Input-Routing Design

**Objective:** specify the minimum AT-compatible controller and keyboard-device
model before code changes.

**Required contract:** controller-owned status/input/output state; ports
`0x60` and `0x64`; command byte; self/interface test responses; keyboard-port
enable/disable; output-port A20/reset behavior; output-buffer delivery and
IRQ1; and a bounded keyboard command subset. The design names the exact
translation boundary from platform host events to guest scan-code bytes and
the QDKEYB INT 09h handoff. Its first implementation slice is exactly:

- controller commands `0x20`, `0x60`, `0xAA`, `0xAB`, `0xAD`, `0xAE`, `0xD0`,
  and `0xD1`;
- `0x60`/`0x64` OBF/IBF status and command-byte behavior;
- keyboard-device ACK, reset, enable, disable, and identify responses; and
- one default-profile host-key mapper that emits the approved fixed scan-code
  form into the controller.

**Approved S1 topology and ownership:**

```text
platform-normalized host key event
  -> vm_profile_default_keyboard_mapper (layout policy -> set-1 byte)
  -> core_machine_keyboard_submit_scan_code(core_machine, byte)
  -> core KBC FIFO / OBF / IRQ1 through core PIC
  -> QDKEYB INT 09h reads port 0x60
  -> QDKEYB BIOS scan-code translation and BDA buffer
  -> BIOS INT 16h
```

- Platform normalizes native events into an owner-neutral event; it does not
  choose BIOS AX values, guest scan-code set, keyboard layout, or BDA policy.
- The default profile owns the host-layout-to-fixed-set-1 mapping. T192 admits
  make bytes only; break-byte, set switching, and controller translation stay
  deferred.
- `core_machine_keyboard_submit_scan_code` is the one explicit running-machine
  ingress contract. It reaches the core-owned KBC internally and never returns
  a mutable KBC pointer to VM composition or a profile.
- KBC owns a fixed 16-byte keyboard FIFO. A full FIFO returns
  `NTVDM64_STATUS_INVALID_STATE` to the mapper and drops no already queued
  byte; the mapper records no second queue or retry thread. S2 probes the
  result, while S3 keeps current interactive behavior by submitting
  synchronously at the product boundary.
- KBC writes IRQ1 directly through the core-owned PIC binding. Its `0xd1`
  output-port operation changes core A20 state and requests a core reset through
  internal machine callbacks. VM supplies neither a PIC nor host reset policy.
- QDKEYB owns only BIOS scan-code-to-AX/BDA behavior. Its INT 09h handler reads
  the KBC data port, updates the BDA, and acknowledges IRQ1; it never receives
  a host event or owns controller state.

**Status and command rules:** `OBF` means the FIFO has a byte available;
`IBF` is asserted only during synchronous command/data processing and is clear
when the I/O write returns. The first slice returns command-byte `0x05` after
reset (IRQ1 plus system flag, with translation clear), self test `0x55`,
interface test `0x00`, and output-port bit 1 as A20.
For a `0xd1` output-port write, bit 0 clear requests core reset and bit 1 sets
A20. Keyboard commands return `0xfa` ACK: `0xff` additionally returns `0xaa`,
`0xf4` enables scanning, `0xf5` disables it, and `0xf2` returns `0xab, 0x83`.
Unsupported keyboard commands return `0xfe` RESEND. AUX state/IRQ12 never
appears in the first-slice status byte.

**Non-goals:** PS/2 mouse, USB/HID emulation, vendor-specific 8042 commands,
controller translation, scan-code-set switching, AUX mouse/IRQ12, controller
timing emulation, or direct host policy in `core/machine`.

**Bochs boundary:** Bochs confirms why controller registers, byte queues,
keyboard-device responses, and IRQ state belong together in the controller
state machine. It does not authorize a combined controller/host-input/GUI
device. AUX state and IRQ12 are deliberately absent from this slice and require
a new task with their own port and DOS probes.

**Exit:** S2 and S3 may start only with an explicit one-route migration plan,
unsupported-command result, queue-overflow policy, and focused controller/BIOS
probes. No executable is produced.

#### S2: Core 8042 Controller Slice

**Objective:** replace the KBC placeholder with the T192 S1 controller model and
its core-owned queues, ports, status, command state, A20/reset callback, and
IRQ1 provider.

**Owned surface:** `src/core/machine/kbc.*`, the necessary core-machine
contracts, and focused core port tests. `core_machine` binds its owned
PIC/RAM/CPU-reset services directly; VM supplies neither callbacks nor
controller state.

**Acceptance:** owned port vectors establish command/data ordering, status
bits, output-buffer consumption, command-byte handling, selected tests,
enabled/disabled keyboard port, A20/reset callback results, keyboard ACK/reset/
enable/disable/identify responses, and deterministic overflow behavior. The
old direct QDKEYB host route remains temporarily active until S3, so no host
input behavior changes in this subtask.

**Stop:** a requirement to store host handles/threads in KBC, introduce a
second keyboard queue outside KBC, or alter current Console interaction.

#### S3: Default-Profile Handoff

**Objective:** move default-profile keyboard delivery from direct host-to-QDKEYB
injection to the one T192 S2 KBC ingress/output/INT 09h route.

**Owned surface:** the default-profile host-key mapper, VM composition's
keyboard binding, and QDKEYB firmware. The mapper owns host-layout policy and
emits guest scan-code bytes; KBC owns controller state; QDKEYB obtains bytes
through the controller's BIOS interrupt route and translates them into the BDA
buffer. The task removes the former direct host-to-QDKEYB injection after
proving equivalent retained input behavior.

**Acceptance:** host key input follows the declared route to BIOS INT 16h
through KBC output and IRQ1; modifier/toggle state has one documented owner;
two-session isolation passes; and the existing Console/debugger and DOS-prompt
keyboard checks stay unchanged from the user's perspective.

**Artifact:** `build/output/nxvm_0_5_0192.exe` after focused KBC/profile
probes and retained runnable regressions.

**Stop:** any need to change Console key grammar, debugger control keys, or
keyboard UX without separate owner approval.

### T193: VADP CGA Text Controller Slice

#### S1: Capability And Text-State Design

**Objective:** define the first truthful VADP hardware contract as a CGA text
controller slice, and define its boundary from default-profile INT 10h
firmware.

**Required contract:** core-owned CRTC register state and indexed `0x3d4` /
`0x3d5` behavior; `0x3d8` mode, `0x3d9` color, and `0x3da` status behavior;
the B8000 text-memory window; cursor/page interpretation; dirty generation;
and a copied text scanout snapshot. S1 selects the exact supported CRTC index
subset and defines all other indexes/ports as unsupported or inert. It also
defines the memory-mapping/write-notification path so direct guest port or
VRAM writes and QDCGA INT 10h mode setup update the one VADP state. QDCGA
maintains BIOS data-area policy and requests VADP configuration; it is not a
controller or a second snapshot owner.

**Text-controller scope:** S1 fixes the CRTC text index subset `0x0a`/`0x0b`
(cursor shape), `0x0c`/`0x0d` (display start address/page), and `0x0e`/`0x0f`
(cursor location). It fixes the `0x3d8` text-mode bits required by the existing
mode set, and declares other CRTC indexes and graphics interpretations
unsupported/inert. `0x3da` returns stable `0x00`, matching the retained
no-raster baseline; it does not claim retrace or raster timing. A B8000 text
write, CRTC/mode/color register write, or QDCGA mode/cursor/page operation
advances the same VADP dirty generation. Snapshot capture copies that state
and visible text cells; it cannot retain guest VRAM.

**Boundary:** VADP is a bounded text-only video core, not a renderer.
Composition alone translates its copied snapshot into a `core_platform` frame.
QDCGA supplies BIOS INT 10h policy and BDA maintenance, then invokes the VADP
contract; it never implements port state, VRAM mapping, dirty tracking, or
scanout copying itself.

**Non-goals:** CGA graphics pixels, palette rasterization, EGA/VGA registers,
sequencer/graphics/attribute controllers, DAC, planar VRAM/latches, raster
timing, video BIOS expansion, external ROMs, or a new renderer.

**Exit:** S2 has an owner map with no raw mutable VADP pointer retained by
the profile, an approved text probe corpus, and an explicit unsupported-graphics
result. No executable is produced.

#### S2: Text Capability Migration

**Objective:** make the approved CGA text contract real: core VADP owns CRTC/
mode/color/status state, B8000 mapping, dirty tracking, and copied text
snapshots while QDCGA remains the default-profile INT 10h firmware provider.

**Owned surface:** `src/core/machine/vadp.*`, the display provider contract as
needed, VM composition binding, QDCGA, and focused text-mode probes. Platform
continues to consume copied frames and never observes a mutable VADP object.

**Acceptance:** guest `0x3d4`/`0x3d5`, `0x3d8`, `0x3d9`, `0x3da`, and B8000
writes exercise the one VADP state and produce a bounded copied text snapshot;
QDCGA mode/cursor/page setup targets that same state; current Console/window
text rendering is unchanged; default profile no longer directly borrows mutable
VADP storage; and unsupported graphics requests take the T193 S1-defined
result rather than being silently represented as text.

**Artifact:** `build/output/nxvm_0_5_0193.exe` with text probes, provider
isolation, retained Console/window, FDD/HDD boot, and DOS-prompt evidence.

**Stop:** any demand for a generic graphics implementation, direct platform
access to guest memory, or profile-owned duplicate presentation state.

#### S3: Graphics Capability Admission Review

**Objective:** decide, from T193 S2 evidence and owned DOS program probes,
whether a bounded graphics-device design is justified. The first possible
target is CGA `320x200x4`, not an unspecified generic graphics mode.

**Non-goals:** implementation by default. This subtask may only create the
next implementation task's breakdown; it cannot claim graphics compatibility
or start a broad CGA/EGA/VGA project.

**Exit:** either a narrowly specified future implementation task for CGA
`320x200x4` with B8000 graphics memory addressing, exact mode/color-register
semantics, palette, scanout pixel format, frame probes, and regression budget;
or a documented deferral that preserves the S2 unsupported-graphics result.
EGA/VGA admission is a separate later decision, split by register family and
real-program probe: mapping windows, sequencer, graphics/attribute controller,
DAC, then planar/latch behavior. No executable is produced.

## Execution Order

`T191 S1 -> T191 S2 -> T192 S1 -> T192 S2 -> T192 S3 -> T193 S1 -> T193 S2
-> T193 S3`.

T192 remains after T191 so all device evidence uses one consistent source/probe
policy. T192 S2 cannot redirect host input; T192 S3 is the sole route-changing
subtask. T193 S2 cannot broaden into graphics; T193 S3 is the only admission
point for that separate design work.
