# M5 T13 S6 Presentation-Type Cutover

`core/platform/presentation.h` now uses the shared core text snapshot and no
longer includes a VDM header. `vdm/platform/dos_minimal_presentation.c` obtains
the complete VDM-private snapshot, then copies only characters and attributes
into the shared presentation snapshot. PIT ticks and pending keyboard IRQ stay
private to `vdm/machine`; no conversion assigns them to a core field.

GCC built all targets. DOS-minimal presentation and profile smokes passed,
followed by the finite CPU probe, FDD-backed execution-context lifecycle, and
FDD/HDD reset-vector smoke using the approved local fixtures. The retained
NXVM Console and delayed debugger gates passed. No raw recorder ran.
