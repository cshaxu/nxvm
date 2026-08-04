# Project Status

## Current Work

**Active subtask:** none. **M5 T208 complete:** the default PC/AT profile is
an immutable declaration interpreted by composition. Its ROM mapping, firmware
service order, CMOS/FDC port configuration, and FDC IRQ/DMA route no longer
reside in session wiring or VM device implementation. The retained startup,
Console, debugger, and DOS-prompt paths are unchanged. See
[`pc-at-profile.md`](../architecture/pc-at-profile.md).

M5 remains open. `M5 Td S2` clarified documentation entry points only; it did
not close any M5 technical or verification item. The earlier documentation-only
commit labels `T208` and `T209` are reclassified as `M5 Td S1` and `M5 Td S2`;
they do not reserve numeric implementation task identifiers.
`M5 Td S3` records the ROI-ordered NXVM PC/AT workstream in `TODO.md`; it does
not activate an implementation task or change M5 closure state.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M11 | Not started | [Roadmap](roadmap.md) |

## Latest Technical Baseline

M5 T208 retained NXVM behavior while making the default PC/AT topology
declarative. The recorded developer artifact is `nxvm_0_5_0208.exe`; its
historical evidence is summarized in [M5 History](../history/m5.md).

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)

Completed task-level records are historical context, not current authority.
