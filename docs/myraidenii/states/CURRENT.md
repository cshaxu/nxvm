# MyRaidenII Status

## Current Work

M1 T3 remains open after accepted S1--S9:

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
| S9 | Finite initial COP upload/trigger inventory and first strict rejection | `456cde1fb` |

The owner approved advancing Video's prerequisite inside T3 and delegates
routine same-T S planning. B04 now has an actual modeled periodic receiver;
full Video rendering and DMA qualification remain queued work. The next S
receives B07/B08 COP operation contracts/evidence and real-set startup proof.
It may not copy reference code, fabricate results or silently close unresolved
COP obligations. No further approval is needed for ordinary same-T ordering.

The next batch is S10: COP upload/register protocol and the three-member
memory-transfer family, with one safe scheduler service point and independent
L2 functional proof. S9's 22 uploaded identifiers and first `0x5205` rejection
are the finite receiving evidence; other COP/results/DMA and full-game
qualification remain open. Ordinary same-T S planning is owner-delegated.

## Active S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S10; sequential coordinator/executor roles. |
| Admission And Approval | Owner's standing implementation approval and delegated same-T S ordering; S9 accepted at 574b92492. Owner additionally approved the documented Shared x86-core callback-fault publication repair and eight affected NXVM receiving artifacts in S10. |
| Objective | Consume S9's complete upload/register protocol and three-member memory-transfer family (5205/5a05/f205): define an explicit L2 functional contract, preserve runtime upload state and perform real bus effects at the existing Core scheduler safe point, with exact failure/reset behavior. |
| Non-goals | Other COP numeric/collision/DMA-trigger algorithms, generic microinstruction interpreter, imported reference code/table, embedded game upload fingerprint, Lib/Emulator/MyNES source changes, physical COP latency or game qualification. |
| Reference Baseline | 574b92492; S9's 22-ID finite initialization ledger and S8 Board/Video; actual public DMA/report_fault/attachment contracts; pinned MAME unit as L2 behavioral research only. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../etc/evidence/t3-s9-cop-initialization-protocol.md. |
| Files And ABI Surface | Existing MyRaidenII cop.c/cop.h, Board attachment integration, existing board_bus_smoke.c and records; Shared x86-core machine.c/machine_scheduler.c, existing executor_run_smoke.c and two manifests; eight current NXVM PC EXEs and receiving receipt. No public API change. MyRaidenII estimate production +180--260/-30--70, tests +140--230/-0--15; Shared production +25--40/-5--15, tests +40--80. |
| Applicable Rules | Architecture one CPU/Core timeline, bus transaction and COP state owner; Coding Lib vocabulary and cohesive private helpers, no duplicate error state/queue; source policy factual L2 contracts and synthetic independent oracles, no reference implementation/table import; Execution complete units/actual review/P/cleanup; Document authority boundaries. |
| Verification | Real configured Board guest issues command writes; independent transfer/upload/register/offset/busy/reset and read/write refusal/prefix assertions. Existing Core executor unit covers before/after callbacks at exact instruction/tick boundaries. Both stable Ninja/ccache graphs run complete units; rebuild/deploy eight current PC EXEs, PE/hash identity and selected receiving integrations, unchanged MyNES/INI/media guards. Optional owner-ROM prefix is diagnostic only. Doc/whitespace checks and actual diff review. |
| Expected Markers | Runtime uploaded words never embedded; one pending hardware operation; DMA through public Core after CPU bus release; failure reaches existing Core fault path without shadow error flag; unimplemented IDs still reject. |
| Asset Needs | Owned build/myraidenii-t3-s10 for research/logs/probes and receiving receipts, 64MiB output/5-minute no-progress/90-minute phase budgets; optional ROM probe limited to 2M instructions/120 seconds/64KiB neutral summary. Eight existing PC receiving EXEs use their approved firmware embedding and overlay media; MyNES pair and protected inputs unchanged. MyRaidenII has no executable release yet. |
| Reporting Requirements | Confirm exact operation/transport/failure model and final estimates before code; report actual delta, retained owners, full dual-width units, any real-ROM result separately, evidence hashes and cleanup. |
| Stop Conditions | Stop affected work before any public API or Shared change outside the approved fault-publication repair, reference implementation/table import, raw protected output, copied upload fingerprint, invented hardware-exact time or unrelated Video/Sound expansion. |
| Exit Criteria | Entire admitted protocol/transfer batch has actual dual-width bus/failure/reset proof, final full units pass and artifacts are correctly scoped, actual diff reviewed, evidence/cleanup and P commits pushed. Other B07/B08 obligations and T3 stay open. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance with autonomous ordinary same-T planning and minimal cohesive code/tests. |
| Similar-Issue Sweep | All three transfer forms and all register lanes share admission/service/failure/reset; preserve unknown-command rejection and existing RAM boundary tests. No nested DMA, paused-debug operational access or mirrored fault status. |

S10 implementation is delivered pending sequential coordinator review. The
owner-approved Shared fault-publication repair is pushed as P1 f2ca9ae01;
eight affected PC receiving releases are pushed as P2 990ff6eda. Both full
unit suites pass687/687 and14 selected receiving integrations pass per width.
The first incomplete x64 attempt remains explicitly recorded. No public API,
Shared state, Lib/Emulator/MyNES source or protected input changes occur.
[Review/evidence](../etc/evidence/t3-s10-cop-transfer-review.md) and
[results/identities](../etc/evidence/t3-s10-results.json) retain exact scope,
line counts, boundaries and safe cleanup. No S10 owner-ROM/gameplay pass is
claimed. T3 remains open for the other complete B07/B08 families and startup.
