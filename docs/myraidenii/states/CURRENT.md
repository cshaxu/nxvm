# MyRaidenII Status

## Current Work

No MyRaidenII S is active. M1 T3 remains open after accepted S1--S8:

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
