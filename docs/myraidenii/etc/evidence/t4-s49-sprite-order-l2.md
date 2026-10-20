# T4 S49 Raiden II Sprite Ordering L2 Model

## Evidence and boundary

The owner authorizes mature external emulator implementations as L2 evidence.
The inspected MAME SEI25X/RISE1X sprite-device source is BSD-3-Clause and
documents a compatible eight-byte entry: word one carries the tile code and
the device processes entries from the end of its bounded object image toward
the beginning.  Its selected drawing loop omits code zero.

This task independently implements only those two finite behavioral facts in
the already-existing MyRaidenII Video compositor.  It imports no source text,
tables, ROM content or MAME runtime behavior.

## Local model

Video is still the sole compositor.  Sprite Buffer remains the sole owner of
live/latched object RAM, and Graphics remains the sole pixel reader.  The
compositor scans its copied 4 KiB snapshot backward in eight-byte units.  A
zero tile code is a disabled object even when its other descriptor fields are
non-zero.  Therefore an earlier object address paints later than a later
address when both have the same priority.

Sprite extension/banking, blend/shadow, object protection and real-game
frame comparison remain deferred; no claim of physical timing accuracy is
made.
