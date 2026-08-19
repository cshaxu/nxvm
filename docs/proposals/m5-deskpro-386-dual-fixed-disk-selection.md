# M5 DeskPro 386 Dual Fixed-Disk Selection

## Purpose

Close one observable functional gap in the selected Compaq WD 40 MB controller:
the DeskPro fixed-disk interface supports two installed drives, designated C:
and D:, whereas the current Core personality admits only the master/media slot.

## Evidence And Boundary

The DeskPro 386 Volume II fixed-disk material documents the two-drive C:/D:
configuration. The controller-matched WD1003-IWH observation confirms the same
task-file family: reset/diagnostic/initialize behavior, no Identify support,
and drive-select wiring. This task is original for the selected-machine fact
and reference-derived only where it uses the observed WD controller shape.
No external text, code, firmware, or media is copied.

## Approach

Extend the existing Core-owned HDC topology from one immutable media binding to
two fixed slots. The current Drive/Head task-file bit selects the slot; every
command, status/error result, IRQ14 acknowledgement, and SRST path continues
through the same HDC owner and port provider. VM composition supplies copied
bindings and retains backing ownership. Model-40 keeps its current single
40 MB startup image unless a declared second project-owned fixture is supplied.

## Limits

This does not infer Long/Format/ECC/recovery, physical-sector semantics,
service duration, electrical IRQ/DRQ timing, firmware setup behavior, or a
physical/L3 result. It does not broaden default ATA or represent generic IDE
as Compaq behavior.
