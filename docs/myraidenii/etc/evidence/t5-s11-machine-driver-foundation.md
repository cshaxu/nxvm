# M1 T5 S11 Machine Driver Foundation

## Boundary

`myraidenii_machine` is the one Machine-owned adapter between a constructed
Board and the existing Emulator Machine callback contract. It borrows the
Board and its Core executor, takes ownership of a successfully injected Lib
PCM stream and owns the S10 PCM bridge,
and provides the one descriptor copied into Emulator Machine.

The caller retains stream ownership when adapter construction fails; this
keeps creation atomic without an invented rollback path.  Once construction
succeeds, Machine owns the stream and may retain itself only when the existing
stream teardown reports failure, so that teardown can be retried safely.

The adapter runs one bounded Core instruction quantum, lets Core select any
existing deadline advance, copies the complete Board RGB32 frame into only the
granted Window frame slot, and never retains that slot. It has no product
configuration, ROM loader, controls policy, extra worker, renderer, guest
clock, PC dependency or direct Core-to-UI/Lib route.

## Lifecycle

Reset first discards staged/submitted audio through the existing bridge, then
uses the existing Core reset. Stop requests the existing Core stop and cancels
the existing Lib writable wait. A normal control wake cancels that same wait
and yields without manufacturing guest time. Heartbeat activation owns only stream
activation; deactivation clears the bridge before disabling the stream. A
native stream teardown failure retains the Machine object for a later retry.

The fixed 256-instruction quantum is L1 host-control containment. It does not
publish elapsed guest ticks, derive a host clock, or claim timing fidelity.

## Output Geometry And Grades

The existing 320 by 240 RGB32 Board output constants moved from private
`video.h` to public `graphics_interface.h`: Machine must name exactly the
existing Board frame contract without depending on a private Core header. This
is an ownership correction, not a new video model or geometry claim.

No manual PDF formula or constant is adopted, so S11 has no L3 claim. No
external emulator behavior is adopted, so it has no L2 claim. The quantum and
ordinary adapter control behavior are L1 local mechanisms.

## Regression

`test/myraidenii/unit/machine/driver_smoke.c` compiles the actual driver and
audio bridge with only lower-boundary fakes. It verifies descriptor completion,
reset, one bounded run, executor callback rendezvous, RGB32 geometry/frame
copy, heartbeat clear/deactivation, stop/cancel and destroy failure retry.
It neither creates a native endpoint nor simulates a ROM/Product path.

The complete MyRaidenII unit selection passes 8/8 on each cached x64 and x86
tree. Documentation governance and whitespace checks pass. No desktop,
endpoint, external-ROM, controls, Product/Session or playable-game result is
inferred from these unit checks.
