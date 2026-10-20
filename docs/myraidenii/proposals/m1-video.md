# Video Qualification

## Objective And Boundary

Implement graphics decoding/decryption, tile layers, sprites, priorities,
scroll/raster behavior and complete native frames in MyRaidenII Core. Requires
the selected graphics inventory and board bus; presentation remains the
existing Emulator/Lib path, not a new renderer or window owner.

T4 S1 has completed the initial convergence review in the
[Video/Sprite ledger](../etc/evidence/t4-s1-video-sprite-convergence.md).
It establishes that the next selected-set stop, `0x6ce--0x6cf`, has an L2
address/direction fact but no lawful implementable flag contract.  The first
later implementation packet must therefore establish the complete
sprite-control consumer contract before it admits any receiver.  No generic
no-op, RAM fallback, copied reference algorithm or Shared workaround is an
eligible substitute.

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
