# MyRaidenII Status

## Active S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: MyRaidenII M1 T3 S1. One session performs sequential coordinator and executor roles. |
| Admission And Approval | T2 closed at `18eb3fd6d`; the owner-approved MyRaidenII objective and queued board/COP candidate admit the next numeric task. This S is a bounded planning/freeze batch only. |
| Objective | Freeze the complete board/COP implementation ledger: selected board ranges, byte lanes, ROM/RAM/bank behavior, interrupt/vector/acknowledgement, input edges, sound latches, COP/DMA operations, public Machine seams and finite proofs. |
| Non-goals | No production C/CMake, Shared/API change, ROM extraction/import, MAME code/table reuse, game bypass, video/audio/Product implementation, executable or gameplay qualification. |
| Reference Baseline | Clean `18eb3fd6d`; T1 F01--F13 source/set freeze, archived T2 public V30/Machine qualification, current X86 Core interfaces and the selected MAME reference only as L2 research. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../history/M1-T3-board-cop.md; ../etc/evidence/t3-s1-board-contract.md; ../etc/t1-freeze-ledger.md; ../etc/research/references.md. |
| Files And ABI Surface | MyRaidenII task records and a new evidence ledger only. Expected production/test/CMake/artifact delta0; expected documentation/evidence +120--260 lines. No ABI surface. |
| Applicable Rules | Execution task-scale ledger and active-packet requirements; Architecture Product->Machine->Core and sole X86 Core timeline; Coding source/test ownership; source policy prohibits game/third-party import. |
| Verification | Read actual public memory/ROM/port/attachment interfaces and existing owner tests; validate every required board category has one owner and named proof; documentation/links/whitespace checks. No runtime build or ROM test is claimed by this planning S. |
| Expected Markers | A finite B01+ board ledger, explicit source grades, no unknown map/COP operation silently omitted, and a subsequent implementation S plan whose tests refine existing fixtures where responsibilities overlap. |
| Asset Needs | Existing read-only metadata and pinned source records only. No access, copy, extraction or change to external game assets. |
| Reporting Requirements | Record the map/operation universe, source confidence, exact public seams, unresolved contract gaps, S sequencing and test-reuse decision. Distinguish observed fact from L2 model. |
| Stop Conditions | Stop for a required Shared public-contract expansion, missing lawful source, ambiguity that cannot be modeled honestly, or any need to import/alter ROM data. Do not invent a hardware map or COP result. |
| Exit Criteria | A complete finite T3 ledger and follow-on S plan exist; every task boundary has an owner and proof receiver; no source/API/asset change occurred; records pass documentation governance and actual diff review. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance; reuse existing coverage where it owns the same responsibility and do not inflate tests through redundant copies. |
| Similar-Issue Sweep | Compare all board categories against existing neutral Machine memory/port/attachment contracts and current test fixtures; record true contract gaps once rather than creating a board-specific parallel runner or duplicate fixture family. |

M1 T1 and T2 are closed in [history](../history/). T3 is active at its planning
S1 only; no runnable MyRaidenII executable or gameplay result is claimed.
