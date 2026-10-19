# MyRaidenII Status

## Current Work

No MyRaidenII S is active. M1 T3 remains open after accepted S1--S4:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5` |

S4 established that the real selected set reaches COP configuration, register,
command and DMA-configuration address families through the actual V30 bus.
Their current ordinary-RAM fallback is not accepted as a device implementation.
No command result, result read, DMA trigger, runnable product, desktop, audio
or gameplay result follows from this evidence-only receiver. The next T3 S
must explicitly own only the local COP register boundary; command and DMA
semantics remain separate receiving work in the [T3 history](../history/M1-T3-board-cop.md).
