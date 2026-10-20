# T4 S48 Raiden II Sprite Compositor L2 Model

## Decision

The owner directs this product to use mature external emulator behavior as L2
evidence where physical L3 material is unavailable. The selected rendering
model is independently written: one Video compositor consumes the existing
latched object-RAM snapshot and the S47 decoded sprite stream.

## Adopted L2 Facts

The read-only MAME Raiden II video reference records four object priorities,
the transparent sprite pen `15`, 64 sprite palette groups at the base of the
board palette, and a five-phase order: before Background, between Background
and Midground, between Midground and Foreground, and before Text. These facts
are used as behavior only. No LGPL source text, expressions, tables or driver
code is copied into this repository.

## Local Ownership

`myraidenii_core_video_frame_copy()` is still the only Core pixel compositor.
It copies the Sprite Buffer's latched bytes before mutating the destination,
then performs all four sprite phases inside the existing tile-layer sequence.
The Sprite Buffer remains the sole owner of live and latched object RAM;
Graphics remains the sole packed-pixel reader; Board remains the only public
frame caller and always supplies its owned buffer.

The direct Video unit path may pass a null Sprite Buffer solely to exercise
pre-existing tile composition. It is not a Board, Machine or Product render
path and does not create a second production compositor.

## Bounded Semantics

Each non-zero eight-byte entry is decoded through the existing descriptor
decoder. Width, height, signed coordinates, descriptor order and both flip
bits select 16-by-16 decoded tiles. Out-of-frame pixels clip; pen 15 leaves
the already-composed pixel unchanged. Tile extension/banking, blend/shadow,
sprite flags and physical timing remain explicitly deferred.

## Regression Coverage

The Core graphics smoke uses synthetic bytes only. It proves priority zero is
behind Background, a live post-latch mutation cannot alter the frame, and
each of priorities one through three overlays its defined later phase. It also
proves both flips select the opposite tile corner, pen 15 is transparent, a
sprite beginning at x=-1 clips while its remaining pixel renders, and an
upfront failed frame request preserves the caller's prior destination. Existing
tile-only composition tests remain in the same executable.
