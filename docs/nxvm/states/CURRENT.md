# Project Status

## Current Work

M5 T538 S1 is active: inventory and reproduce all eight deployed EXE/INI boot
pairs before grouped root-cause repair. [Proposal](../proposals/m5-deployed-boot-pairs.md)
owns the bounded task; [ledger](../etc/evidence/t538-boot-pairs.md) owns coverage.
The five unrelated [Queue](QUEUE.md) candidates remain pending.

## Active Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T538 S1. T537 is the highest previously allocated NXVM identifier; later cross-product T41 records are preserved, not reusable NXVM numbering capacity. |
| Admission And Approval | Owner request on 2026-09-25 admits a new NXVM task to audit and repair all EXE/INI pairs after observed exits and hangs following start. |
| Objective | Freeze all eight deployed pairs and external inputs; reproduce their startup through the real product path and classify every result before planning batch repairs. |
| Non-goals | No MyNES or Shared changes; no firmware substitution, synthetic boot success, changes to owner media masters, or broad CPU qualification. |
| Reference Baseline | Clean a2418ae57; eight 0535 EXEs from c5b17f671 and four current owner INIs. Previous unit/integration passes do not prove these deployed launch cases. |
| Candidate Proposal | [Deployed boot pairs](../proposals/m5-deployed-boot-pairs.md). |
| Files And ABI Surface | NXVM docs, product-only integration harness and its NXVM CMake registration if required for real EXE observation. No public ABI or shared corpus change. |
| Applicable Rules | Shared EXECUTION, ARCHITECTURE, CODING and DOCUMENT; NXVM design ARCHITECTURE, CODING, UI, ROADMAP; source-and-research policy. Unique state owners, real adjacent INI inputs, external readonly/overlay media, no manufactured timing claim. |
| Verification | Hash all eight EXEs/four INIs and referenced inputs; validate PE width and file resolution; bounded real-process launches with start and checkpoint/exit capture for all eight cases. Model 40 x86 gets a 300-second diagnostic rerun because its 180-second capture already starts the DOS title; all other bounds remain 180 seconds. Run full x64/x86 repository-only units if test code changes; documentation gate and actual-diff review. |
| Expected Markers | Each pair has a reproducible disposition: DOS prompt/date/installer checkpoint, pre-start error/exit, post-start error/exit, or timeout with last observable state. No unobserved case is called passing. |
| Asset Needs | Existing lawful external profiles-nxvm/media-nxvm inputs only; record hashes without copying payloads. Preserve masters and owner INIs. |
| Reporting Requirements | Report eight-case inventory, exact deployed launch results, repeated failure classes, test limitations and next repair batch. S1 is diagnosis, not task closure. |
| Stop Conditions | Missing lawful assets, required Shared change, destructive media access, or observer perturbing the launch invalidates the affected result; report before changing authority. |
| Exit Criteria | All eight baseline pairs are inventoried and attempted through the production launch path with classified evidence; repair batches have owners and regressions; required checks pass and complete S1 delivery is reviewed and pushed. |
| Original Owner Request | Admit a new NXVM T to audit and repair every NXVM EXE/INI pairing so start reaches DOS or its installer; some deployed profiles exit or hang. |
| Similar-Issue Sweep | Examine all four profiles and both widths for path resolution, build-fixed assets, parse/construction failures, lifecycle dispatch, display/input and guest boot checkpoints; no single-profile success substitutes for the matrix. |

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
