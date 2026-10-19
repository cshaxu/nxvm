# MyRaidenII Status

## Current Work

M1 T3 remains open after accepted S1--S7:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |

## Active S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S8; sequential coordinator/executor roles. |
| Admission And Approval | Owner approves advancing the Video prerequisite on 2026-10-19 and explicitly delegates routine same-T S ordering. S7 accepted at d7ccb5978. |
| Objective | Bring forward B04's Video-owned VBlank source within T3: freeze an explicit L2 raster timing model, integrate its deadline/advance with the existing Core timeline and deliver actual Board IRQ/reset proof. |
| Non-goals | Full Video rendering/decryption, COP command/result/DMA algorithms, ROM patches, host timing, Shared changes, playable game or executable claim. |
| Reference Baseline | d7ccb5978; B01--B09 ledger; S7 owner transfer; pinned MAME raiden2 driver c1425a1af00b88aa07477b1b23a30ed5e5f856ff as factual L2 timing/wiring research only; actual public Core scheduler. |
| Candidate Proposal | ../proposals/m1-board-cop.md and ../proposals/m1-video.md; ../history/M1-T3-board-cop.md. |
| Files And ABI Surface | MyRaidenII Core private video timing files, Board composition, existing board_bus_smoke.c, local CMake and task/design records. No new public function or Shared ABI. Initial estimate production +60--100/-0--5 and existing tests +50--90/-0--5; actual +109 test lines include real ISR/reset setup, and three local CMake lines bind the existing test executable to build-unit-tests after stale-test detection. |
| Applicable Rules | Execution packet/full units/P review and cleanup; Architecture single guest timeline and Video state owner; Coding Lib vocabulary/private cohesive files/no duplicate path; source policy independent implementation of explicitly graded hardware facts, no source/table translation; Document current authority. |
| Verification | Actual configured V30 Board route: first edge, subsequent period, IRQ delivery and acknowledgement, reset phase, halted deadline progression and sliced versus whole progression. Both stable Ninja/ccache graphs build and pass complete repository-only units; doc/whitespace gates. No real-ROM startup claim. |
| Expected Markers | Exactly one Video phase and Board IRQ latch; deterministic earliest deadline and repeated IRQ delivery through public Machine; nominal pacing never claimed verified physical timing. |
| Asset Needs | No ROM use. Owned build/myraidenii-t3-s8 for pinned research and test logs, 60-minute phase/5-minute no-progress/64MiB output budgets. Retain hashes/results then clean after processes exit; caches remain. |
| Reporting Requirements | Before-code timing facts/model and source/test estimates; report actual delta, full dual-width results, retained owners, source hash, cleanup and remaining COP/Video work. |
| Stop Conditions | Stop affected work before Shared change, imported algorithm/table, fabricated COP response, ungraded physical timing claim or full Video expansion; resolve owner-local defects within scope. |
| Exit Criteria | B04 periodic receiver passes actual Board/Core tests on both widths, source grade and exact raster model recorded, complete units pass, actual diff reviewed and P commits pushed; T3 remains open. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance; owner delegates same-T ordering and approves advancing Video prerequisites. |
| Similar-Issue Sweep | Verify reset/ack/deadline/advance all use the one Video phase and Board interrupt latch; no host timer, frame counter clock or COP periodic workaround. |
