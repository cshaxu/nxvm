# T580 S72 — PC110 CMOS persistence investigation

## Baseline and actual ownership

S71 (`f23baaa30`, accepted in `e10c0c945`) established that the unchanged
TFT firmware writes configuration in Setup and continues to INT 19h after
a real second wake on both host widths. That proves an in-process path, not
configuration persistence across an application exit and a fresh process.

The current PC110 build embeds an optional 128-byte CMOS input; the existing
Machine preparation copies its configuration bytes beginning at 0Eh into the
board RTC. The guest then owns the live RTC values. Machine teardown destroys
Core before invoking the existing `BOARD_DETACHED` notification, so that
notification cannot safely read the RTC. Emulator Product first shuts down the
worker before calling the private Machine destroy callback, providing a
potential quiescent save boundary. The Board and Machine now expose a
quiescent capture of configuration bytes 0Eh onward, leaving clock/status
registers alone. Machine rejects a still-bound Emulator and returns a copied
64- or 128-byte image with the first 14 bytes zero. Caller-owned quiescence is
also required for any direct unbound execution. The existing Board CMOS and
PC110 construction tests passed on x64 and x86 after the addition, including
classic/extended sizes, guest-written bytes and invalid buffer sizes. The
PC110 test also destroys its first machine, constructs another with the
captured bytes as its seed and verifies an identical second capture. This is
an in-memory construction test, not an application restart or file test.
The changed X86-NXVM source/test manifests verify. This is a partial
working-tree change, not yet a persistence route or delivery.

The App uses executable-adjacent `mypc110-dstn.cmos` and
`mypc110-tft.cmos` files, which are ignored as user-owned runtime state.
Opening in append mode creates an empty first-run file without truncating an
existing one; an empty file explicitly means uninitialized, exactly 128 bytes
are loaded as the selected seed, and other sizes or actual open/read errors
fail. Setup writes remain owned by the guest RTC. After successful machine
binding, the Product teardown calls the App save callback only after worker
shutdown and unbinding. A save failure propagates without releasing the
Machine. A failed initial bind does not save.

The current Storage writer does not provide atomic replace. Save uses its
existing truncate/write/close operations and reports their failure, but a
partial write or interruption can damage the previous CMOS file. No claim of
crash-safe persistence is made; this is a real limitation to resolve at the
Storage boundary if stronger durability is required, not grounds for a new
App-specific recovery framework. Empty files left by an interrupted first
creation are indistinguishable from the intentional initial sentinel.

This file currently preserves configuration RAM only: RTC calendar, alarm and
control registers 00h--0Dh are not restored. The existing RTC starts its own
calendar on construction, so guest-set date/time across a process restart and
elapsed host-off time are not qualified by S72. They remain part of the open
T580 RTC/product work and must not be presented as complete battery-backed
clock behavior.

The focused construction and Product tests passed on both widths, including
separate writer/reader processes, malformed nonempty image and missing-parent
I/O failures, and failed save without Machine destruction. The new registered
TFT/DSTN product integration tests passed 2/2 per width using real preview
executables and fresh private directories; they check the executable-adjacent
state file across two product launches, without running guest Setup.

On x64 the unchanged TFT BIOS executed the real Setup/save/reset, the second
physical wake and INT 19h handoff; the diagnostic captured guest-owned CMOS
through the same App save operation and exited zero. The guest log SHA-256 is
`5B504AAE81C59684E630CC9BF683C3C66A0BCA37F678D555AFBE45BA33BBC1AA`;
the disposable captured image hashes to
`1AA230E26CAF01D2B762EBAB9CD95659EE7FFFA2A73543EB53CF88E8AC02E90C`.
A separate x64 process reloaded that image and executed the original BIOS's
checksum routine: its computed and stored values were both `03BCh`; the
process exited zero and its log SHA-256 is
`8F12140987007BF555B755BE3DC78BE88C88C0A120BFD0130EF8008590C5175F`.
This proves the original BIOS consumed that saved configuration, not that a
guest operating system booted. The corresponding x86 run also exited zero;
its guest log SHA-256 is
`AD3FF17281047BE24488E154C8BB0322BA534CF5B54270E34DD2E48D2E733D97`.
Its captured CMOS hash and independent BIOS checksum result are identical to
x64, and its restore log has the same SHA-256 as the x64 restore log.
The long Setup replays ran with the diagnostic built before the later
`--restore-cmos`-only observation branch was added. That branch selects a
separate argument mode; the Setup path continues to use the same firmware
bytes and save operation. The fresh-process checksum checks used the final
diagnostic build. The earlier executable is not represented as final bytes.

The affected X86-NXVM unit selection passed 233/233 per width; the PC110
unit selection passed 12/12 per width. Both TFT and DSTN file-backed Product
integration cases passed on each width. Eight existing PC binaries were
rebuilt in the stable optimized Release trees and passed all eight profile INI
identity receivers. Their paths, widths and hashes are in the
[S72 artifact inventory](t580-s72-artifacts.json); all four existing INI
hashes were unchanged. The repository's specialized gates passed on x64.
No full repository or desktop qualification is claimed.

The counted tracked and newly added source/test changes, excluding manifests,
documentation and generated artifacts, are production +154/-4 and tests
+227/-17, net +360 lines. The increase is the actual CMOS capture and App
file owner, lifecycle integration, real-firmware receiver and focused
failure/real-process tests. Existing RTC remains the only live state owner;
the PC110 test's copied in-memory seed and the persisted file are inputs and
outputs, not a second running RTC. No Lib or Emulator files changed.

## Scope still to prove

Verify guest-written Setup data and its subsequent consumer on both widths,
and finish the relevant regression, artifact and actual-change review. Preserve
the input seed and owner files. DSTN and TFT panel identity, real guest/media
boot and desktop color are separate T580 rows.
