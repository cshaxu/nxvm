# M5 T580 S61 — PC110 extended-CMOS seed disposition

## Current implementation boundary

`x86-nxvm/machine` accepts an optional 64-byte CMOS payload and deliberately
copies only configuration indices `0Eh` through `3Fh` into the existing Board
RTC defaults.  The Board already decodes the PC110's extended `70h`/`71h`
address range through `7Fh`, but its default-byte capacity is also exactly the
same 50 entries.  Therefore the current route cannot initialize an extended
configuration byte such as `44h`, `72h`, `73h`, or `78h`.

This is a real representational gap, not a live-RTC ownership gap: Core still
owns RTC registers and the Board remains the sole default-image consumer.

## Source and asset audit

Open-Source-PC110's `Discovery/Live-Dump/README.md` says that a full 128-byte
live CMOS capture was recorded in `ports_indexed.json`; its PS2 disassembly
record independently identifies persisted settings in the `40h`–`7Fh` range
and a `40h`/`41h` checksum.  The locally pinned repository commit `02ccf2e`
does not contain that JSON file or any raw `.cmos`/NVRAM payload.  Its public
documentation supplies several individual field meanings, but not enough
authoritative bytes or checksum semantics to construct a faithful complete
seed.

The external `nxvm-assets` PC110 profiles contain the selected BIOS and font
payloads, but no CMOS seed.  The source policy requires such a protected/raw
payload to remain external with provenance and a hash; this task did not fetch,
create, mutate or commit one.

## Disposition

Do not expand the generic seed from 64 to 128 bytes yet.  Such an interface
change would be mechanically straightforward but would add an unconsumed
capability while silently inviting invented bytes.  A later receiver may be
admitted only after an owner-supplied or lawfully retained exact 128-byte
capture is placed in the external PC110 profile archive with provenance and
SHA-256.  It must then extend the existing single seed/default route once,
preserve Core's live-RTC ownership, test `0Eh`–`7Fh` coverage and prove the
selected firmware consumes the supplied configuration.

Normal Easy-Setup persistence, guest boot, PC Card mapping and product
publication remain unqualified.  No production/test code changed in S61.

## Existing Easy-Setup diagnostic boundary

The existing `mypc110-real-firmware-post-integration --setup` mode also cannot
substitute for the missing seed.  It deliberately requires a scratch-only,
232,185-byte loader comparison oracle, which S55 removed after recording its
hash and results.  Current external assets contain the selected BIOS/font but
not that oracle or an authentic internal-disk image.  Reconstructing it from a
different execution path would turn this source/asset audit into a new
diagnostic design and still would not establish CMOS persistence.  It is not
performed here.

## Candidate-media cross-check

The local PC110-QEMU `disks/Personaware-disk.img` is not a substitute for the
missing PC110 primary-media input.  It is 4,194,304 bytes (8,192 sectors), and
its MBR declares an active FAT12 partition beginning at LBA 32 with 8,160
sectors.  The source-recorded SunDisk geometry permits only 7,872 total
sectors.  Truncating the QEMU image to the SunDisk capacity would therefore
truncate its declared partition, even though its discarded tail currently
contains zero bytes.  PC110-EMU's `Disk1.img` is larger still at 8,011,776
bytes.  Neither is an admissible primary-media input for the exact geometry
route.

## Easy-Setup colour disposition

The reported blue ERROR/Easy-Setup background is not evidence that the Window
renderer has red and blue reversed.  The current F65535 path expands the VGA
DAC's three sequential components into `0x00RRGGBB`, and the Machine forwards
that value unchanged to the Window graphics frame.  The S57 captured value
`BE9E96` is therefore the intended muted rose in that contract, not blue.

PC110-QEMU's current `target-i386/pc110post.c` is useful **L2 diagnostic
evidence**, not code to import: it records that its direct Easy-Setup loader
skips the real ROM pre-entry initialization, initially selecting a blue theme,
then overwrites nine bytes of the loaded guest image to force its rose theme.
That guest-image modification is precisely the kind of BIOS/Setup interception
this task excludes.  PC110-EMU similarly contains presentation-specific palette
role overrides and cannot establish a hardware register default.

Consequently there is no justified F65535 RGB/BGR change or palette constant
to make here.  The useful next proof is a current-BIOS frame capture with the
actual pre-entry sequence intact, paired with the exact extended CMOS/default
image that selects the real theme if that sequence consumes it.  The absent
128-byte capture remains the first concrete prerequisite.  A stale pre-S57
preview is also a possible explanation until a current frame is captured; no
published PC110 artifact exists from which to infer otherwise.
