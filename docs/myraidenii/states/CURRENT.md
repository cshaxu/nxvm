# MyRaidenII Status

## Current Work

No MyRaidenII S is active. M1 T3 remains open after accepted S1--S7:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |

S7 found no source-grade COP command/result/DMA model. T3 retains the future
bus-model owner but may not invent it; the periodic VBlank/video-DMA discovery
receiver is the later M1 Video task. The next implementation step requires an
owner decision to advance that Video task before resuming T3, or source-grade
COP evidence that admits a finite local model.
