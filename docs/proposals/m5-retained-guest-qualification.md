# Retained Machine Guest And Windows Qualification

## Goal

Qualify the retained XT, AT, DeskPro 386 and default PC/AT machines with their
actual hardware and firmware. No machine is replaced or gated on Standard selection.

## Current Product Context

The completed cutover establishes four product bindings: one selected Profile and external
BYOB asset root at build time, one adjacent NXVM.ini for runtime media and
presentation, and deployment only in `assets/binary/<profile>/`. This task
extends that exact production route; it does not use retired YAML/catalog
construction or manufacture firmware/CMOS/media substitutes.

## Dependencies And Batches

Freeze each machine's board/firmware/media set and complete the needed CPU/controller
contracts first. Record finite per-machine checkpoints for DOS boot, keyboard,
media access, protected-mode/extender transitions, then Windows 3.1 installation,
Standard Mode and 386 Enhanced Mode lifecycle in that dependency order.
Record any unsupported prerequisite explicitly; do not broaden the hardware
configuration dynamically to pass a later checkpoint.

After the configuration cutover, use NXVM.ini and external readonly
masters/overlays through the production asset route. Preserve the original
integration universe with an explicit per-row migration map. Move scenarios to
the new structure and loader without removing their machine coverage or weakening
predicates. All four existing families keep boot and controller regressions;
Windows/protected-mode scenarios apply only to capable hardware, not to XT
as though every machine were interchangeable. PC110 qualification follows its
separate hardware implementation proposal.

## Exit

Each selected scenario reaches its accepted checkpoint with input and lifecycle
proof. Budgets contain tests but are not success predicates. Required unit and
integration suites pass, artifacts are reproducible, and the final hardware/
timing audit agrees with actual code. Protected media is never committed.
