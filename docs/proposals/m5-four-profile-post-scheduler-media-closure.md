# M5 Four-Profile Post-Scheduler Media Closure

## Purpose

Qualify the supported IBM 5160 Model 268, IBM 5170 Model 339, DeskPro 386
Model 40, and `default-at` DOS-media matrix against the completed Core
event-deadline scheduler.  This is the compatibility-consumer receiver for the
remaining T499 Release replay work; it does not reopen the scheduler or create
a profile-specific progression path.

## Frozen Universe

The universe is the nine rows frozen by the retained
[four-profile matrix](m5-four-profile-supported-media-matrix.md):

- IBM 5160 Model 268 / 8088 / 360 KB;
- IBM 5170 Model 339 / 80286 / 360 KB and 1.2 MB;
- DeskPro 386 Model 40 / 80386 / 1.2 MB; and
- `default-at` / 8086 / 360 KB, 80186 / 720 KB, 80286 / 360 KB, and 80386 /
  1.2 MB and 1.44 MB.

A row succeeds only at `dos-prompt`, `date-input`, or `installer-ready`.
A finite retirement or wall-clock budget is diagnostic containment, never a
terminal.  An unavailable owner-managed input may remain an explicit
external-input boundary only when its receiver, missing authority and effect on
the row are recorded.

## Boundary And Ownership

Core remains the sole owner of guest time, controller state, deadline order,
memory and CPU state.  VM owns session construction, frozen media/firmware
selection and copied presentation only.  The runner may observe a copied Core
deadline and request the existing Core advancement operation; it may not inject
ticks, use host elapsed time as guest time, or add a profile-specific wake path.

DeskPro ROM diagnostics, FDD topology and board configuration are not scheduler
facts.  The existing T499 result proves continuous scheduler progress through
the first 8272A command.  This task must locate the earliest remaining owner
from original material and read-only emulator cross-checks before changing
behavior.  It must not add a second-ready drive, BIOS workaround, F1 shortcut,
or guessed controller duration.

## Subtasks

1. **Release matrix baseline.** Rebuild the current stripped Release artifact,
   replay every available row with the existing local-only BYOB probe, and
   record the first semantic terminal or earliest non-terminal boundary.
2. **DeskPro diagnostic ledger.** Reconcile the complete selected Model 40 ROM
   POST/FDD/keyboard/configuration boundary against original documentation and
   read-only 86Box, MAME, PCjs, Bochs and QEMU implementations.  Produce a
   finite List 1/List 2 before any repair.
3. **One-owner repair batch or explicit transfer.** Implement the full
   source-qualified owner-local batch for the selected failure class, or
   transfer the whole underdetermined class to its earliest owner.  No row-by-
   row compatibility patch is permitted.
4. **Matrix closure.** Rebuild the stripped Release artifact and replay all
   nine rows.  Run the complete configured gate.  Record every terminal and
   every legitimate external-input boundary without retaining firmware/media
   bytes, paths, hashes or traces.

## Exit Criteria

- Every frozen row has a fresh Release semantic terminal or a source-gated,
  explicitly named external-input boundary.
- The Model 40 result reaches its earliest source-backed FDD/diagnostic
  boundary without a scheduler, firmware, FDD, DMA, PIC, VM or profile
  shortcut; any remaining blocker has one named receiver.
- No new VM/profile time writer, second scheduler, media cache, controller
  owner or compatibility forwarding path exists.
- The complete configured gate remains green, and the task artifact is an
  optimized stripped Release executable with the runtime debugger retained.
