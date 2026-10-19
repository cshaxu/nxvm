# MyRaidenII Status

## Active S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S6. One session performs sequential coordinator and executor roles. |
| Admission And Approval | S5 is accepted at `5079476c4`. The owner granted standing approval for bounded, in-scope MyRaidenII S work; this evidence-only receiver consumes the remaining S4 B07 command/result/DMA gap. |
| Objective | Freeze the finite evidence ledger needed to model the actual RAIDEN II COP command/result/DMA operations, without deriving or implementing behavior before that evidence exists. |
| Non-goals | No production COP algorithm, command-result fabrication, DMA transfer, Shared API, ROM/source commit, MAME code/table import, video/sound work, Product/executable/gameplay claim or broad reverse engineering. |
| Reference Baseline | Accepted S4 observation `../etc/evidence/t3-s4-cop-observation.md`; accepted S5 `../etc/evidence/t3-s5-cop-register-boundary.md`; frozen B01--B09 ledger and source policy. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../history/M1-T3-board-cop.md; ../etc/evidence/t3-s4-cop-observation.md. |
| Files And ABI Surface | MyRaidenII evidence/current/history only, plus disposable task-owned probe/research roots under `build/` if needed. No tracked production/test/Shared/asset path unless a separately approved packet revision is required. |
| Applicable Rules | Execution evidence/temporary-artifact/closure rules; Architecture one guest timeline and inward dependency direction; Coding no test-only production path; source policy external-ROM/read-only/MAME-research limits. |
| Verification | Reproduce only neutral, bounded facts through the actual V30/Core path: command/result family reachability, ordering category, trigger family and terminal checkpoint. Pin every observation to source grade and an exact receiving owner; clean every temporary root with the governed tool. Documentation governance and whitespace checks pass before P. |
| Expected Markers | A finite command/result/DMA ledger with no invented semantics, raw protected bytes, values, trace, screenshots, source/table text or silently broadened address owner. |
| Asset Needs | Read-only owner-supplied `O:\assets\raidenii\raiden2.zip`, verified against the committed inventory before use; no asset copy persists after the probe. |
| Reporting Requirements | Report observation budget, neutral aggregate facts, source grade, every retained unknown and receiving S; distinguish a reached address from a qualified command result or startup. |
| Stop Conditions | Stop before modeling a command, accepting a command result, implementing DMA, retaining a ROM/protected trace, importing MAME code/table, changing Shared contracts or treating a budget terminal as game progress. |
| Exit Criteria | The B07 command/result/DMA universe has a durable finite ledger: each member is directly observed, explicitly not yet reachable with reason, or assigned to a named next owner; all temporary payload is removed and required documentation checks pass. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance; preserve real hardware ownership, avoid test bloat and do not import MAME behavior. |
| Similar-Issue Sweep | Reconcile all S4 B07 ranges (`0x476..0x47e`, `0x4a0..0x4a6`, `0x4c0..0x4c6`, `0x500`, `0x580..0x5b5`, `0x6fc`, `0x6fe`) rather than promoting one reached address to a complete COP claim. |

T3 S1--S5 are accepted in [history](../history/M1-T3-board-cop.md). S6 is an
evidence-only receiver; it cannot claim an operation model, a boot, a Product
or gameplay result.
