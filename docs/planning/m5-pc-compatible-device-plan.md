# M5 PC-Compatible Device Plan

## Purpose

This plan decomposes three retained core devices whose current implementations
are deliberately incomplete: the 8254 PIT read-back command, the Intel
8042-compatible keyboard-controller path, and the video-adapter state used by
the default VM profile. Each design activity is the `S1` subtask of its
corresponding behavior task; no design-only task number is created. It does
not add a DOS runner, change the NXVM Console grammar, or claim CGA/EGA/VGA
compatibility before an owned probe demonstrates it.

## Current Facts

- `core/machine/pit.*` owns the one PIT state and normal counter operation,
  but does not implement the 8254 read-back command.
- `core/machine/kbc.*` is an `0x64` status placeholder. Host input currently
  travels through the VM keyboard transport directly to the default-profile
  QDKEYB provider; that is BIOS-keyboard behavior, not 8042 compatibility.
- `core/machine/vadp.*` owns a resettable state container only. Default-profile
  QDCGA implements INT 10h text services and creates the current text snapshot
  from BDA and `0xb8000`; it is not a generic hardware video adapter.
- `core_machine` remains the sole owner of PIT/KBC/VADP storage and lifecycle.
  VM composition may bind profile providers and host transports, but may not
  create a second device, reset it directly, or retain mutable aliases.

## Shared Constraints

- Each task uses primary device documentation for behavior, then writes small,
  owned port or snapshot probes. Bochs 2.6 compatibility sources may be read
  only to compare state-machine decomposition and observable behavior: no
  source, global device manager, GUI callback path, C++ plugin structure, or
  build/runtime dependency is copied or linked into the product.
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

Bochs' keyboard device is a useful model for controller command state, input
and output buffers, IRQ requests, and keyboard-device responses. ntvdm64 keeps
those as a core KBC state machine but does not adopt Bochs' combined global
device/GUI/plugin model. Its explicit route is:

```text
platform host event
  -> vm/default-profile keyboard mapper
  -> core KBC controller and byte queues
  -> QDKEYB BIOS INT 09h
  -> BIOS data-area keyboard buffer
  -> BIOS INT 16h
```

Bochs' VGA split likewise informs, but does not define, the VADP boundary:
`core/machine/vadp` owns register state, guest video-memory mapping, dirty
state, and copied scanout snapshots; VM composition converts a snapshot to a
platform frame; core/vm platform code owns renderer/window/Console behavior
and never accesses guest VRAM. QDCGA is BIOS INT 10h firmware, not the video
controller.

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
rewrite.

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

**Non-goals:** PS/2 mouse, USB/HID emulation, vendor-specific 8042 commands,
controller translation, scan-code-set switching, AUX mouse/IRQ12, controller
timing emulation, or direct host policy in `core/machine`.

**Exit:** S2 and S3 may start only with an explicit one-route migration plan,
unsupported-command result, queue-overflow policy, and focused controller/BIOS
probes. No executable is produced.

#### S2: Core 8042 Controller Slice

**Objective:** replace the KBC placeholder with the T192 S1 controller model and
its core-owned queues, ports, status, command state, A20/reset callback, and
IRQ1 provider.

**Owned surface:** `src/core/machine/kbc.*`, the necessary core-machine
provider contracts, and focused core port tests. VM supplies only frozen
callbacks for IRQ/A20/reset effects; it does not own controller state.

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
`320x200x4` with memory addressing, mode/color register, palette, scanout
pixel format, frame probes, and regression budget; or a documented deferral
that preserves the S2 unsupported-graphics result. EGA/VGA admission is a
separate later decision, split by register family and real-program probe. No
executable is produced.

## Execution Order

`T191 S1 -> T191 S2 -> T192 S1 -> T192 S2 -> T192 S3 -> T193 S1 -> T193 S2
-> T193 S3`.

T192 remains after T191 so all device evidence uses one consistent source/probe
policy. T192 S2 cannot redirect host input; T192 S3 is the sole route-changing
subtask. T193 S2 cannot broaden into graphics; T193 S3 is the only admission
point for that separate design work.
