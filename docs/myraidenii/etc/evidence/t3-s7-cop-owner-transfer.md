# T3 S7: COP Command Evidence And Owner Transfer

## Scope

This record consumes the complete unresolved B07 COP command/result/DMA
ledger received from S6.  It decides only whether a bounded production model
can be admitted.  It adds no source, table, ROM data, test vector or runtime
behavior.

## Reviewed Reference And Grade

The following three MAME files were fetched only into the task-owned,
disposable `build/myraidenii-t3-s7-research/` directory from MAME commit
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`:

| File | SHA-256 | Grade and usable fact |
| --- | --- | --- |
| `src/mame/seibu/raiden2.cpp` | `FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A` | L2 address-map and screen/VBlank ownership hint only. |
| `src/mame/seibu/seibucop.cpp` | `7E9763D7D1026E82473F135AEE26317DFD8E930C5F1AC257780CD813484A75A9` | L2 implementation reference only; no algorithm or table is imported. |
| `src/mame/seibu/seibucop.h` | `69857872F1E68D807442FA6018562A29390ACE4E0D66ECF5D86ABE080829E962` | L2 interface/ownership hint only. |

The reference itself records incomplete documentation, TODO/FIXME items,
per-game behavior and hardcoded trigger handling.  It therefore cannot
establish source-grade command decoding, result values, DMA semantics,
latency, stall behavior or hardware-exact timing.  The MyRaidenII source
policy consequently forbids translating it into a local COP model.

## Disposition

| Unresolved S6 member | Evidence status | Next owner | Required receiving proof |
| --- | --- | --- | --- |
| Command writes at the admitted command receiver | S6 proves arrival; no source-grade decoder/model exists. | Resume T3 after source-grade evidence or a later Video checkpoint exposes a finite, independently reproducible contract. | A finite command/result contract plus Board regression through the real bus. |
| Result-register reads | No observed reads in S6's 5M/20M bounded baselines; no source-grade result contract. | Resume T3 under the same condition. | Real-set checkpoint reaches a named read and an independent expected result/failure fact. |
| DMA trigger writes | No observed triggers in S6's 5M/20M bounded baselines; only L2 map ownership exists. | MyRaidenII M1 Video task for video-owned DMA/event discovery; T3 remains owner of any eventual COP bus model. | Video task proves the board event/consumer relation without fabricating COP results; T3 then receives a finite DMA contract. |
| Periodic main-CPU event | L2 reference wiring associates VBlank with the screen/video receiver, not the COP command receiver.  Exact cadence and board behavior remain unproved. | MyRaidenII M1 Video task. | One deterministic Video-owned periodic-event contract and a ROM-free timing regression; no COP shortcut. |

This transfer does not claim that the missing periodic event is the exact
cause of S6's absent result/DMA observations.  It only prevents T3 from
inventing a COP timer or result path that belongs to neither its evidence nor
Video's future receiver.

## Completion And Cleanup

No source-grade original Seibu COP specification or hardware observation was
found in the reviewed materials.  S7 therefore admits no production batch.
The research root contained three non-protected reference files totalling
245,735 bytes and is removed by the governed cleanup tool after this record
is written.  This record retains only revision identities, hashes and neutral
conclusions; it contains no copied implementation, command values, ROM data
or raw trace.
