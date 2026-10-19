# MyRaidenII Status

## Active S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S5. One session performs sequential coordinator and executor roles. |
| Admission And Approval | S4 is accepted at `77110cf04`. The owner has standing approval for bounded, in-scope MyRaidenII S tasks; this packet consumes the S4 B07 address ledger only. |
| Objective | Replace the ordinary-RAM fallback for observed COP configuration, register and command-write addresses with one local MyRaidenII Core COP owner; preserve reset and lane correctness without inventing command results. |
| Non-goals | No MAME source/table/code import, ROM commit, command-result emulation, result-read fabrication, DMA copy/scheduler, Shared API, video/sound/Product execution, generic arcade framework, executable or gameplay claim. |
| Reference Baseline | S4 evidence at `../etc/evidence/t3-s4-cop-observation.md`; frozen B01--B09 ledger; accepted S4 `77110cf04`; existing X86 Core replacement route and single guest timeline. |
| Candidate Proposal | ../proposals/m1-board-cop.md; ../history/M1-T3-board-cop.md; ../etc/evidence/t3-s4-cop-observation.md. |
| Files And ABI Surface | `src/myraidenii/core` COP/Board source and private header as necessary; matching `test/myraidenii/unit/core` Board-owned test only; MyRaidenII evidence/history/current updates. No Shared or external asset path. |
| Applicable Rules | Execution full-unit closure/one-target P rules; Architecture one guest timeline and inward dependency direction; Coding Types/test-owner rules; source policy external-ROM/research prohibition. |
| Verification | ROM-free public Machine tests prove each observed B07 write family is a replacement device route rather than ordinary RAM, preserves required byte/word lane behavior, resets deterministically and tears down safely. Unimplemented command/result routes return one explicit documented status. Focused x64/x86 tests, complete repository unit suites and documentation governance pass before P. |
| Expected Markers | One local COP state owner, Board as sole route composer, no generic device layer, no duplicated RAM/COP path, and a finite S6 handoff for operation semantics/DMA. |
| Asset Needs | None. All S5 tests use synthetic ROM-free Board construction. |
| Reporting Requirements | Report exact owned register families, unsupported route behavior, retained S6 items, source/test added/removed/net lines and verification scope; do not call focused proof a game boot. |
| Stop Conditions | Stop before inferring command meanings/return values, introducing a second clock or DMA loop, changing Shared APIs, or expanding into video/sound addresses. |
| Exit Criteria | All S4-observed COP families are owned by explicit local routes rather than RAM; reset/byte-lane/error behavior is proven; no unimplemented command/result behavior is hidden; required tests/gates pass and no temporary payload remains. |
| Original Owner Request | Implement MyRaidenII under single-session dual-role governance; preserve real hardware ownership, avoid test bloat and do not import MAME behavior. |
| Similar-Issue Sweep | Inspect every S4-observed B07 family and all Board routes at their boundaries for the same RAM-fallback shape; classify non-COP `0x600--0x6ce` accesses to their existing/future Video/Bank owners without absorbing them. |

T3 S1--S4 are accepted in [history](../history/M1-T3-board-cop.md). S5 is a
strict local COP-boundary delivery; no runnable MyRaidenII executable or
gameplay result is claimed.
