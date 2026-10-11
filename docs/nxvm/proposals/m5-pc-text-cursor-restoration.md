# Default-PC Text Cursor Restoration

## Objective

Restore the default NXVM App's guest text-mode cursor in both Console and
Window presentation. The existing Core -> Emulator -> Lib copied-frame route
must remain the single route.

## Evidence And Boundary

My5160, My5170 and MyDeskPro386 render correctly through the same Core,
Emulator and Lib presentation chain. The default App's project-owned `INT 10h`
mode 03h handler writes CRTC offset but not maximum raster or cursor shape.
With reset's one-raster baseline, its intended cursor scanlines 6--7 are
invalid and the existing frame contract correctly hides them.

The repair belongs only to `src/app-nxvm/firmware/video.asm` and an App-owned
guest-execution receiver. No Shared/Core presenter workaround is allowed.

## S Tasks

### S1: Cursor fact trace — complete

Recorded the divergent default firmware initialization and rejected the
Core-wide candidate before build or commit.

### S2: Default firmware repair — complete

Initialize mode 03h CRTC maximum raster and cursor shape, then prove the
result from actual default guest firmware execution. The handler now writes
maximum raster 7 and cursor scanlines 6--7; the new receiver passes x64/x86.

### S3: Default-App qualification and delivery — active

The default App x64/x86 artifacts are rebuilt, architecture-verified and each
complete 56-route App selection passes. Await owner Console/Window validation.

## Exit Criteria

- The default firmware establishes valid text-cell and cursor-raster geometry.
- Its App-owned regression observes a visible cursor from guest execution.
- The existing Console and Window render route remains unchanged.
- Required x64/x86 tests pass; changed default-App artifacts are rebuilt,
  deployed and PE-verified.
