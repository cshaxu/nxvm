# Project Status

## Current Work

No NXVM implementation task is active. MyNES-hosted M6 T43 S11 is the
owner-approved cross-target documentation reconciliation; its
[packet](../../mynes/states/CURRENT.md) owns admission, not an additional NXVM
packet. [Queue](QUEUE.md) retains five unnumbered candidates.

| Task | Closure |
| --- | --- |
| T41 | Closed by owner acceptance on 2026-09-24. Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b complete S13; its evidence records dual-width units, manifests and x64 integration 20/20. [Evidence](../etc/evidence/m6-t41-s13-softpc-raw-crt-reconciliation.md). |

## Current Technical Baseline

- Four fixed products: XT, AT, Model 40 and default PC/AT. Their eight optimized,
  stripped 0535 x64/x86 EXEs remain in `assets/nxvm/<profile>/`, adjacent to
  the owner INIs. PC110 is not an implemented runnable product.
- The latest receiver rebuild is MyNES-hosted T43 S10, NXVM commit `c5b17f671`.
  [S10 receiver evidence](../etc/evidence/m6-t43-s10-receiving-artifacts.md)
  owns current hashes and complete x64/x86 unit results, 335/335 each. The
  earlier T535 artifact record is historical, not the current hash authority.
- The canonical Shared baseline is `75099c178`: `src/{lib,common,x86}`,
  matching test roots and `test/register.cmake`. Common remains ISA-neutral;
  x86 owns the optional assembler and Debug CLI. S10 rebuilt all receivers.
- This documentation reconciliation changes neither executable inputs nor
  INI/media/firmware. The existing artifacts remain current; no new runtime
  or integration result is claimed.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve T41 and earlier deliveries, including their historical pending stages.
They do not reopen a task or supersede the current baseline. T41's hosting and
cross-target attribution are preserved as recorded, not inferred from commit
prefixes or renumbered during this cleanup.
