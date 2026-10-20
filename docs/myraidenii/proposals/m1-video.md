# Video Qualification

## Objective And Boundary

Implement graphics decoding/decryption, tile layers, sprites, priorities,
scroll/raster behavior and complete native frames in MyRaidenII Core. Requires
the selected graphics inventory and board bus; presentation remains the
existing Emulator/Lib path, not a new renderer or window owner.

T4 S1 has completed the initial convergence review in the
[Video/Sprite ledger](../etc/evidence/t4-s1-video-sprite-convergence.md).
The owner subsequently approved a bounded L2 model derived from other
emulators' observable behavior, without importing their code, tables or
decryption implementation. T4 S4 therefore admits only the write-only
`0x6ce--0x6cf` 16-bit sprite-flags latch, its V30 byte order and reset state.
It does not claim sprite decoding, decryption, buffering, palette, layers or
frame publication.

T4 S2's [source disposition](../etc/evidence/t4-s2-video-source-disposition.md)
confirms that the currently available original/operator material and permitted
external asset metadata do not independently fill that contract. Its source
grade remains recorded; the owner's later L2 authorization is narrow and does
not upgrade it to an original-hardware claim.

T4 S3 separately rejects a publicly available FPGA implementation as that
source: its own [provenance record](../etc/evidence/t4-s3-third-party-provenance.md)
states that it derives the relevant behavior from MAME source and it is not an
independent hardware oracle or import candidate.

S37 proves only that one synthetic graphics-region mutation affects a later
reference presentation. S38's paired synthetic/control experiment establishes
that, under the current no-payload black-box containment, a full-screen
time-advancing difference masks any finite element coordinate/value result.
See the [S38 boundary](../etc/evidence/t4-s38-synthetic-graphics-oracle.md).
It is not authority to guess a tile layout, bitplane order, attributes,
priority, scroll or decryption rule. Decoder admission therefore requires
original board documentation/measurement or a separately approved, stable
black-box isolation method.

S39's bounded original-document search found no provenance-qualified graphics
contract in accessible operation-manual/catalog material. Its
[disposition](../etc/evidence/t4-s39-original-graphics-source-search.md)
confirms that the current evidence gap remains; it does not turn a missing
source into an implementation license.

## Work And Verification

T3 S8 brings forward the Video-owned L2 raster phase/VBlank-to-Board receiver
under the owner's 2026-10-19 ordering approval. This task reuses that single
owner and extends its qualified scope; it does not introduce another cadence
or treat the early IRQ proof as complete video/DMA/game qualification.

Freeze register/layout evidence and model grades before implementation. Verify
ROM-free pixel/layer cases, bounds and invalid-source behavior, then compare
named real-game scenes including overlap, scrolling and stage transitions.
Distinguish native raster, ROT270 orientation and host scaling/aspect. Use a
small presentation probe as needed without inventing a second host loop.

## Exit And Stops

Complete-frame output and failure/no-new-frame contracts agree through the
Machine adapter and real Window. No stale/partially converted frame is published.
Do not substitute a screenshot or disable encryption/priority effects. Hardware
timing derived from a hand-tuned reference remains explicitly L2.
