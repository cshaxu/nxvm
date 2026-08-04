# Project Status

## Current Work

**Active subtask:** **M5 T212 S3 -- system-image and product regression.** S2
has retired the F2 video portal through the ordinary default-ROM/IVT text
slice and corrected the per-instruction display-copy performance regression.
S3 must now pass the retained full matrix, produce the task artifact, and
record closure evidence without changing NXVM user experience. See [the task
record](m5-t212-video-portal-retirement.md).

**M5 T211 complete:** the default profile's F0
boot-failure portal is retired. The ROM reports an acknowledged failure through
its BDA POST status; the VM session runner consumes it and requests the sole
core stop. See [the task record](m5-t211-boot-failure-portal-retirement.md).

**M5 T210 complete:** the default profile's F1/F3
keyboard portals are retired. The retained input path is KBC -> IRQ1 -> ROM
`INT 09h` -> BDA -> ROM `INT 16h`; platform input still submits only host
events. See [the task record](m5-t210-keyboard-portal-retirement.md).

**M5 T209 complete:** the default profile's QDX
opcode override is replaced with frozen, ROM-origin-limited private `INT`
portals. Standard `INT`/IVT semantics remain intact outside a matching frozen
portal. The default machine remains `80386 + no FPU`; its focused strict-8086
diagnostic stops at a genuine 80186 instruction in the DOS image. See [the
task record](m5-t209-firmware-interrupt-portal.md).

**M5 T208 complete:** the default PC/AT profile is an immutable declaration
interpreted by composition. Its ROM mapping, firmware service order,
CMOS/FDC port configuration, and FDC IRQ/DMA route no longer reside in session
wiring or VM device implementation. The retained startup, Console, debugger,
and DOS-prompt paths are unchanged. See
[`pc-at-profile.md`](../architecture/pc-at-profile.md).

M5 remains open. `M5 Td S2` clarified documentation entry points only; it did
not close any M5 technical or verification item. The earlier documentation-only
commit labels `T208` and `T209` are reclassified as `M5 Td S1` and `M5 Td S2`;
they do not reserve numeric implementation task identifiers.
`M5 Td S3` records the ROI-ordered NXVM PC/AT workstream in `TODO.md`; it does
not activate an implementation task or change M5 closure state.
`M5 Td S4` establishes the required hardware-device verification template:
every admitted device task has S1 contract/port probe, S2 owner-local
implementation, and S3 DOS/system-image regression, with a fixed retained
NXVM matrix and bounded optional Bochs differential rules. It changes no guest
behavior and produces no artifact.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M11 | Not started | [Roadmap](roadmap.md) |

## Latest Technical Baseline

M5 T212 retains NXVM behavior while moving the admitted text `INT 10h` subset
from the F2 portal into default-ROM firmware and bounding runner display-copy
cadence. The recorded developer artifact is `nxvm_0_5_0212.exe`; its historical
evidence is summarized in [M5 History](../history/m5.md).

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)

Completed task-level records are historical context, not current authority.
