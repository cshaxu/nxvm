# M5 T554 — Default-PC Text Cursor Restoration

## Admission

T554 is admitted by owner request on 2026-10-10 after T553 closure. It repairs
the missing cursor only in the default NXVM App.

## S1 — Complete: Cursor fact trace

The initial Core-wide reset-geometry hypothesis was rejected and reverted
before build or commit: My5160, My5170 and MyDeskPro386 use the same
Core-to-Emulator-to-Lib route and render correctly. The default App's
project-owned `INT 10h` mode 03h handler writes CRTC offset but not maximum
raster or cursor shape. Reset's one-raster baseline makes scanlines 6--7
invalid, and the copied-frame contract correctly hides them.

## S2 — Complete: Default firmware repair

The mode-03 handler now writes maximum raster 7 and cursor scanlines 6--7
before setting CRTC offset. The App-owned guest-execution receiver observes
the resulting 8-raster visible cursor and passes on x64/x86. No Core, Lib,
Emulator or other App source changed.

## S3 — Complete: Default-App delivery

Default x64/x86 artifacts are rebuilt and architecture-verified. Each complete
56-route App selection passes. The owner accepted Console and Window cursor
behavior.

## Closure

T554 closes with `b25e2ca8d`. The only production change is the default App's
mode-03 CRTC initialization; both default-App artifacts were rebuilt and
deployed. Core, Lib, Emulator and the other PC Apps remain unchanged.
