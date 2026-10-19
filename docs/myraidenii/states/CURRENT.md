# MyRaidenII Status

## Current Work

No MyRaidenII S is active. M1 T3 remains open after accepted S1--S3:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41` |

S3 uses only MyRaidenII-local Core Board source and its existing ROM-free
Board test family. Both x64 and x86 complete repository unit suites pass
`687/687`; no executable, external-ROM startup, desktop, audio or gameplay
claim follows from that Core delivery. The accepted scope, L2 evidence and
remaining B07--B08 receivers are recorded in the
[T3 history](../history/M1-T3-board-cop.md) and
[S3 evidence](../etc/evidence/t3-s3-interrupt-input-sound.md).

The next T3 S must be explicitly admitted from the Board/COP proposal. It
must retain the frozen B01--B09 ledger, preserve the single X86 Core timeline
and avoid turning an unresolved COP/DMA fact into a game-specific workaround.
