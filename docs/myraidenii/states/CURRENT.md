# MyRaidenII Status

## Current Work

M1 T3 remains open after accepted S1--S8:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |
| S8 | Video-owned L2 raster/VBlank receiver, actual IRQ/reset proof and test build wiring | `34e22a100` |

The owner approved advancing Video's prerequisite inside T3 and delegates
routine same-T S planning. B04 now has an actual modeled periodic receiver;
full Video rendering and DMA qualification remain queued work. The next S
receives B07/B08 COP operation contracts/evidence and real-set startup proof.
It may not copy reference code, fabricate results or silently close unresolved
COP obligations. No further approval is needed for ordinary same-T ordering.

## Active S9 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S9; single-session sequential roles. |
| Admission And Approval | Standing implementation authorization and owner-delegated same-T ordering; accepted S8 at 2a6e13b8d. |
| Objective | Freeze the complete observed COP initialization protocol up to the current explicit command rejection: upload/register access widths, staging counts, finite command identifiers and actual failure/Video-event relation. Assign a bounded next implementation batch from these facts. |
| Non-goals | Production COP semantics, reference-code translation, raw uploaded data/ROM/trace retention, bypass/no-op command, larger continuation budget, Shared changes, game or executable qualification. |
| Reference Baseline | 2a6e13b8d; B07/B08 and S4/S6/S7 evidence; current COP/Board/Video and public observable Core. |
| Candidate Proposal | ../proposals/m1-board-cop.md and ../etc/evidence/t3-s8-video-periodic-event.md. |
| Files And ABI Surface | MyRaidenII evidence/history/current and disposable build/myraidenii-t3-s9 probe only. No tracked source/test/CMake/ABI/artifact change. |
| Applicable Rules | Source policy owner-ROM read-only and neutral bounded aggregates; Architecture one actual Core/Board execution route and no private Shared layout access; Execution finite batch/evidence/cleanup and actual-diff review; Documentation authority boundaries. |
| Verification | Verify archive and two program-lane hashes, compile a diagnostic copy of the current product Core against the existing observable Core library, run the configured real V30 with at most 2M instructions/120 seconds/64KiB summary. Instrument only counts and distinct hardware command IDs before preserving the current rejection. Cross-check upload-family counts and ordering, no fabricated completion. Run doc/whitespace checks; unchanged production/test qualification remains S8's exact source baseline. |
| Expected Markers | A single first-rejection record and complete finite preceding register-family inventory; source/model grades and one next owner per unresolved class. |
| Asset Needs | Owner external archive read-only; extract only verified program lanes under owned build/myraidenii-t3-s9. Temporary probe/source/log cap 32MiB, 5-minute no-progress and 30-minute phase budget. Record source/input/output hashes and remove after owned process exit. |
| Reporting Requirements | Report new facts versus S6, command/instruction budgets versus semantic success, before/after B07/B08 disposition, next implementation scope, source/test zero delta and cleanup. |
| Stop Conditions | Stop affected work before raw protected output, changing production failure semantics, unbounded continuation, reference implementation/table import or Shared modification. |
| Exit Criteria | Verified actual first command rejection with complete preceding protocol inventory and bounded next implementation receiver, no owned temporary payload, doc/actual-diff review and pushed P commits; T3 remains open. |
| Original Owner Request | Implement MyRaidenII using single-session dual-role governance; autonomously choose ordinary same-T S sequencing. |
| Similar-Issue Sweep | Reconcile every S4 configuration family and S6 command/result/DMA family against the current strict boundary; distinguish unsupported result behavior from missing periodic delivery and preserve failure status. |
