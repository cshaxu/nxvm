# M5 T13 S6 Presentation-Type Cutover

`core/platform/presentation.h` now uses the shared core text snapshot and no
longer includes a VDM header. `vdm/platform/dos_minimal_presentation.c` obtains
the complete VDM-private snapshot, whose `text` member embeds the shared core
text snapshot, then copies that child object into the shared presentation
snapshot. PIT ticks and pending keyboard IRQ stay private to `vdm/machine`; no
conversion assigns them to a core field.

## Superseded S6 Boundary Rule

This record remains historical verification evidence for the T13 cutover. The
subsequent M5 design supersedes its ownership direction: `core/platform` and a
product platform must not use a core or product machine snapshot as a platform
type. A product-machine snapshot may embed core text, while the corresponding
product platform owns an independent copied frame. Only the product root
composition may include both contracts and convert the snapshot to the frame.
The pending boundary cutover is governed by the migration map and architecture
contracts; this historical record does not claim that later work is verified.

GCC built all targets. DOS-minimal presentation and profile smokes passed,
followed by the finite CPU probe, FDD-backed execution-context lifecycle, and
FDD/HDD reset-vector smoke using the approved local fixtures. The retained
NXVM Console and delayed debugger gates passed. No raw recorder ran.
