# M5 T580 S60 — PC110 primary ATA geometry

## Source boundary

The local, read-only Open-Source-PC110 research corpus is used as an L2
functional source, not as an imported implementation.  Its
`Discovery/Live-Dump/README.md` live-dump storage record identifies primary
ATA master `0xA0` as a SunDisk SDP3B-4 and records 123 cylinders, two heads
and 32 sectors per track (7,872 sectors).  Its `Discovery/Storage/readme.md`
record independently says that the stock BIOS is CHS-only and obtains that
geometry from fixed configuration rather than ATA IDENTIFY.

The source does not establish a reusable slave geometry, a supplied media
image, CMOS seed bytes, a completed Setup transaction, or guest boot.  Those
facts remain outside this receiver.

## Existing owner and change

`x86-nxvm/machine` already owns the one exact, master-only geometry path:
after inserting slot zero it accepts a construction-supplied CHS geometry only
when the image has exactly that capacity.  MyPC110 declared two ATA slots but
did not select that existing path, so a primary image used generic 16x63
translation.

The App plan now selects the established 123x2x32 master fact.  No new
geometry API, controller, media route, timing value, CMOS seed, slave policy
or BIOS special case is introduced.  The slave remains an independently
attached ATA unit with its existing media-derived behavior.

## Direct proof

The existing MyPC110 construction smoke creates disposable zero-filled local
files only.  A 4,030,464-byte file reaches the real Machine construction and
reports 123 cylinders.  A one-sector file is rejected by the existing exact
geometry guard.  Neither file is a SunDisk image or guest input and both are
removed by the test.

The direct construction route passes on both host widths.  The full labelled
MyPC110 regression set also passes: x64 completes 14/14 in 277.71 seconds and
x86 completes 14/14 in 371.69 seconds.  Each run includes the unchanged
real-firmware FDC-reset and boot-handoff checks, which pass respectively in
45.91/231.73 seconds (x64) and 53.51/318.12 seconds (x86).  These are
non-regression checkpoints only; neither run is described as storage, Setup,
media or guest qualification.

After a fresh Ninja relink of every unit target, the complete repository-only
unit suite passes 714/714 on x64 in 61.36 seconds and 714/714 on x86 in 59.03
seconds.  No executable is published by this receiver: it changes a BYOB
primary-media geometry fact but does not qualify a supplied SunDisk image or
the product release path.
