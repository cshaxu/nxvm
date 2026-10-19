# MyRaidenII Status

## Current Work

No MyRaidenII S is active. M1 T3 remains open after accepted S1--S5:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386` |

S5 made every observed B07 configuration/register and command address an
explicit COP replacement route. It does not model COP commands, results, DMA,
video, sound, a runnable Product or gameplay. The next T3 S must first freeze
the finite command/result/DMA observation batch before implementing its owner.
