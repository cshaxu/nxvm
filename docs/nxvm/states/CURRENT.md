# Project Status

## Current Work

M5 T538 remains open; S1 baseline inventory is accepted, with repair work pending.
[Proposal](../proposals/m5-deployed-boot-pairs.md)
owns the bounded task; [ledger](../etc/evidence/t538-boot-pairs.md) owns coverage.
The five unrelated [Queue](QUEUE.md) candidates remain pending.

## Progress

S1 accepted: implementation `5e6ed82cd` records the complete eight-pair inventory,
real-process diagnostic tool and dual-width full units (335/335 each).
No active S packet remains. T538 stays open: intermittent AT 303 on both widths
and Model 40 x86 guest/presentation progress remain unresolved. Next admission
must consume both residual classes in the linked ledger; no production fix or
new product binary has been delivered.

## Retained Closures

| Task | Closure |
| --- | --- |
| T41 | Closed by owner acceptance on 2026-09-24. Shared b94ea4ffe, NXVM f3a681422 and closure 6a548c43b complete S13; its evidence records dual-width units, manifests and x64 integration 20/20. [Evidence](../etc/evidence/m6-t41-s13-softpc-raw-crt-reconciliation.md). |

## Current Technical Baseline

- Four fixed products: XT, AT, Model 40 and default PC/AT. Their eight optimized,
  stripped 0535 x64/x86 EXEs remain in `assets/nxvm/<profile>/`, adjacent to
  the owner INIs. PC110 is not an implemented runnable product.
- Td S3 removes sixteen 0533/0534 EXEs. Only the eight current 0535 binaries
  remain; their SHA-256 values are unchanged. Earlier binaries are recoverable
  from Git history. The asset guide follows the shared latest-pair rule.
- The latest receiver rebuild is MyNES-hosted T43 S10, NXVM commit `c5b17f671`.
  [S10 receiver evidence](../etc/evidence/m6-t43-s10-receiving-artifacts.md)
  owns current hashes and complete x64/x86 unit results, 335/335 each. The
  earlier T535 artifact record is historical, not the current hash authority.
- The canonical Shared baseline is `75099c178`: `src/{lib,common,x86}`,
  matching test roots and `test/register.cmake`. Common remains ISA-neutral;
  x86 owns the optional assembler and Debug CLI. S10 rebuilt all receivers.
- This retention cleanup changes neither executable inputs nor INI/media/firmware.
  Current artifacts need no rebuild; both product documentation gates and exact
  retained-file inventory/hash checks pass. No new runtime result is claimed.

## Historical Context

[Archived status and packets](../history/M6-T41-and-prior-status-archive.md)
preserve T41 and earlier deliveries, including their historical pending stages.
They do not reopen a task or supersede the current baseline. T41's hosting and
cross-target attribution are preserved as recorded, not inferred from commit
prefixes or renumbered during this cleanup.
