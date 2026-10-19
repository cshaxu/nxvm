# T3 S1: Board/COP Contract And Convergence Ledger

## Scope And Evidence Grade

Baseline is clean `18eb3fd6d`. This is a planning-only S: production, CMake,
tests, manifests, artifacts and external inputs remain unchanged. The selected
set is the frozen original V30 `raiden2` US set 1, with the 13 runtime images
recorded in T1. The downloaded MAME source was the already pinned revision
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, SHA-256
`fdde57f6e9024657f2a9830864523305139562229750b87c967e597324cdb26a`.
It is LGPL research evidence only: it supplies L2 map/behavior leads, not
source, tables, decryption, protection code or a claim of schematic accuracy.

The operator manual establishes vertical two-player JAMMA controls but does not
give a board schematic. NEC V30 semantics/timeline are the accepted T2 public
contracts. No row below promotes a MAME observation into physical timing.

## Frozen Board Ledger

| ID | Finite obligation and factual grade | Sole owner and public route | Required receiver before T3 closure |
| --- | --- | --- | --- |
| B01 | Compose the two verified 512 KiB program lanes; map the reset-visible upper program image and a 128 KiB switchable program window. Window selection has two observed states; exact source-bank selection is L2. | MyRaidenII Core board memory provider; X86 Core immutable ROM mapping for static upper ROM and an ordinary board memory-device provider for the dynamic window. | ROM-free lane/order/reset-fetch vectors; real-set reset fetch and bank transition checkpoint. |
| B02 | Establish low control/COP space, board work RAM, sprite/tile/palette/stack RAM ranges, bank window and static program ROM. The reference exposes these as distinct ranges; video-owned meaning is not implemented by this task. | Core board memory-device routes and ordinary Core RAM, never private Machine storage. | Boundary, hole, byte/word/cross-range and read-only-write-discard vectors through public Machine memory access. |
| B03 | Preserve the V30's byte-lane behavior for program images and low-byte sound communication. High-lane/unmapped behavior is an explicit board result, not an accidental host byte order. | Core board bus callbacks, one physical address/order owner. | ROM-free byte/word reads, odd address/cross-lane reads, accepted and refused lane writes. |
| B04 | Acknowledge the board main interrupt at vector `0x30`; no PC PIC exists. VBlank cadence is not yet claimed because its source belongs to video. | One X86 Core attachment `interrupt_pending`/`interrupt_acknowledge` pair and board pending state. | Explicit board-pulse/acknowledge/reset vectors through an actual configured Machine; no periodic host timer. |
| B05 | Expose two-player directional/two-button, system/coin/start/service and DIP words. Operator manual supports controls; exact address and active-low packing are L2 until independent real-set observation. | Core input-register device; later Product converts host input. | Board register edge/release/reset vectors; Product input integration is a later receiver. |
| B06 | Main-to-sound command/status latches occupy an eight-bit board interface. Sound CPU, YM and OKI behavior remain Sound-task work. | Core board latch callbacks, then later sound component; no Z80 stub or host audio shortcut. | Low-byte handoff, status, repeated command and reset ownership vectors. |
| B07 | COP register/command universe consists of command/data registers, arithmetic/query results, collision/angle/decimal/random helpers, sprite/tile-bank controls and command-triggered memory operations. Every actual selected-set operation needs a source/model/proof disposition. | Dedicated local Core COP component reached only through B02 memory routes. | ROM-free operation vectors plus real-set initialization trace/checkpoint; unsupported operation fails explicitly, never scripted. |
| B08 | COP DMA/sort operations must read/write through Core's public memory transaction route and run on the sole guest timeline. Their physical throughput is unqualified. | Local COP scheduler state plus X86 Core attachment deadline/advance and `x86_core_machine_dma_memory_cycle`; no second clock or executor. | Copy/order/cancel/reset vectors and observed real-set initialization operations. |
| B09 | Reset clears board-owned RAM/latches/COP/deadlines while immutable ROM identity remains valid; teardown removes board routes before Core destruction. | Core board lifecycle/attachment callbacks. | Reset-after-bank/IRQ/COP/latch and teardown/failed-construction vectors. |

## Public Seam Decision

The current X86 Core public surface is sufficient for the first board batches:

- `x86_core_machine_neutral_create()` configures V30 without an App-side
  execution loop.
- memory-device routes own dynamic board regions; observational reads preserve
  device state, and paused inspection does not call an operational read.
- static upper program ROM uses immutable mapping. A writable board bank cannot
  select a different immutable mapping after configuration; therefore B01's
  runtime window is one board-owned provider over immutable App-owned bytes.
  This is the existing extension seam, not a requested Shared API.
- attachment callbacks provide interrupt pending/acknowledge and deterministic
  board advancement. Their owner is the one Core timeline; no vblank clock is
  invented before Video supplies one.
- public DMA memory cycles are the sole future COP transfer route. It does not
  authorize a generic DMA manager or separate device thread.

No public API gap, Lib/Emulator change or third-party code import is admitted
by S1. If S2 discovers that an existing public contract cannot express one
frozen obligation, it must stop before changing Shared code and record the
precise missing fact and smallest contract proposal.

## Follow-on S Plan

1. **S2 — Board construction and bus map.** Create only the local Core board,
   V30 configuration, static/dynamic program routes and board RAM/control
   decode. Use existing Core fixture patterns and add one focused board test
   family for B01--B03/B09; do not duplicate CPU semantic tests.
2. **S3 — Interrupts, inputs and sound latches.** Bind the existing attachment
   and board devices for B04--B06. Test actual public Machine delivery, not
   copied callback structs. No periodic raster or sound synthesis.
3. **S4 — COP/DMA completion.** Freeze actual selected-set COP register writes
   from bounded headless observation, then implement every reached operation
   through B07/B08; each operation gets a source/L2/unsupported disposition.
4. **S5 — Board receiving qualification.** Validate external owner-ROM startup
   through unmodified bus/COP paths, with deterministic named initialization
   checkpoints. This is not attract/video/audio/gameplay acceptance.

Each S must extend an existing fixture where it already owns the same public
route. A new test file is allowed only for a new board-owned responsibility;
separate CPU semantics, generic Core route validation and video/sound behavior
remain with their current owners.

## Explicit Deferred Receivers

Video owns raster cadence, palette/scroll/priority/sprites and frame output.
Sound owns Z80, timers, YM2151, OKI and PCM. Product owns host controls,
configuration and external-ROM preparation. Gameplay owns scene/stage/signature
coverage. These are not omissions from B01--B09 and cannot be used to close
T3 without their own admitted tasks.
