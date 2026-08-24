# Session Input Restart Recovery

## Purpose

Restore the current NXVM session contract after an F9 stop: a subsequent
`START` creates a usable presenter and accepts host keyboard input again.

## Scope

Trace one Win32 session through first input, platform stop report, teardown,
reset, second start and second input. Repair the shared session/platform owner
only if that lifecycle fails, and add one regression that proves both delivery
epochs. Preserve the one input ingress path and opaque run-handle boundary.

## Exclusions

Do not alter guest keyboard scan-set semantics, PIC/KBC behavior, guest media,
Console command grammar, or introduce a second input route.

## Acceptance

The F9-stop/restart sequence accepts ordinary host input in both runs, existing
window/console lifecycle tests remain green, and the current product build and
gate pass.
