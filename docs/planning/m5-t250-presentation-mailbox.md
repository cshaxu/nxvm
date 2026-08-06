# M5 T250: Copied Presentation Mailbox

**Status:** S1 active.

## Goal

Move the synchronized, copied display-frame mailbox contract from VM platform
to `core/platform` without moving guest display capture, renderer ownership,
frame cadence, or Console/window/auto policy. Composition continues to create
the copied frame from a core-machine display snapshot; VM platform continues to
consume it for retained NXVM presentation.

## Subtasks

### S1: Contract And Lifecycle Inventory

Define `core_platform_presentation_mailbox` with initialize, publish, capture,
and finalize lifecycle. Inventory all producers and consumers, copy ownership,
host-thread access, and shutdown order. The mailbox must own only one copied
`core_platform_display_frame`; it must never expose VRAM, VADP state, renderer
handles, display mode, or product policy.

#### S1 Contract

`core/platform/presentation_mailbox_interface.h` will expose one
`core_platform_presentation_mailbox`. It owns an internal lock, active state,
and one complete copied `core_platform_display_frame`. `publish()` and
`capture()` return a factual status; neither retains caller storage, returns a
guest pointer, accesses VADP, or calls a renderer. `finalize()` synchronizes
with in-flight copy operations, marks the mailbox inactive, and clears its
copy. A capture or publish after finalize is rejected.

#### S1 Producer, Consumer, And Teardown Inventory

| Path | Role | Disposition |
| --- | --- | --- |
| `vm/composition/session/display.c` | Captures a core-machine display snapshot, converts it to a frame, increments VM display generation, and publishes the copy. | Keep snapshot conversion and cadence in VM composition; call the moved mailbox publish. |
| `vm/composition/session/session.c` | Creates session storage. | Initialize the core mailbox with other session-owned composition objects. |
| `vm/composition/session/lifecycle.c` | Owns run-handle stop/join before storage teardown. | Keep ordering; finalize mailbox after run-handle join and before core-machine destruction. |
| `vm/platform/win32/w32cdisp.*` and `w32adisp.*` | Console/window renderer consumers. | Capture copied frames only; retain renderer and native-handle ownership in VM platform. |
| `vm/platform/linux/linuxcon.c` | Linux Console renderer consumer. | Capture copied frames only; retain terminal state and rendering policy in VM platform. |
| mailbox and DOS display smokes | Verify independent snapshots and retained guest presentation. | Rename/update to exercise the core contract without changing fixture behavior. |

The mailbox is session-owned composition storage, not a global presentation
service. Platform threads may capture while the run handle is active. Shutdown
must request stop, join every platform thread, finalize the mailbox, then
detach display providers and destroy core machine storage. No other order is
authorized.

**S1 marker:** `M5:T250:S1:PRESENTATION-CONTRACT:OK`.

### S2: Move The Copy Container

Move the mailbox storage and operations to `core/platform`, adapt VM
composition and Win32/Linux renderers to the new contract, and preserve the
current production path. No new frame queue, renderer thread, guest snapshot
route, or display scheduler is permitted.

### S3: Verify The Boundary

Add copy/lifetime/stop coverage and rerun Console/window, text/CGA/EGA, DOS,
debugger, FDD/HDD, current GCC/CTest, and artifact verification. Allocate the
next developer artifact only if runnable source changes.

## Stop Conditions

Stop for owner direction if this requires guest VRAM access outside
`core/machine`, platform renderer ownership in core, display-mode policy,
frame cadence policy, a second mailbox, or a second presentation path.

**S1 marker:** `M5:T250:S1:PRESENTATION-CONTRACT:OK`.
