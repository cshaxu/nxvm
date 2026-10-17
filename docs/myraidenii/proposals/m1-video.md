# Video Qualification

## Objective And Boundary

Implement graphics decoding/decryption, tile layers, sprites, priorities,
scroll/raster behavior and complete native frames in MyRaidenII Core. Requires
the selected graphics inventory and board bus; presentation remains the
existing Emulator/Lib path, not a new renderer or window owner.

## Work And Verification

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
