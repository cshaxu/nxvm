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
