# T3 S8: Video Periodic-Event Receiver

## Admission And Evidence

The owner approved bringing the Video prerequisite forward and delegated
same-T S ordering on 2026-10-19. This refines B04's previously unimplemented
periodic source, without transferring or closing B07/B08 COP obligations.

Read-only research re-fetched MAME driver commit
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, `src/mame/seibu/raiden2.cpp`.
Its SHA-256 remains
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`.
Configuration near lines 1121--1136 associates screen VBlank with the main
interrupt, a nominal 16 MHz V30 and an 8 MHz pixel clock, 512 horizontal
period, 282 vertical period and 240 active lines. The source explicitly calls
the raster hand-tuned. These are L2 numerical/wiring facts, not original
schematic proof. No reference code, algorithm or table is imported.

## Independently Implemented Model

The private Video owner stores only position in the modeled raster period.
On the existing CPU model tick axis, the ratio is two ticks per pixel:

- first VBlank: `240 * 512 * 2 = 245760` ticks;
- period: `282 * 512 * 2 = 288768` ticks;
- nominal rate: `16000000 / 288768`, approximately 55.4078 Hz.

The last value follows the pinned raw geometry, rather than treating its
approximate 55.47 comment as a second timing authority. Reset starts at the
beginning of the active region. This initial phase is an explicit model choice,
not a measured reset-to-raster relationship. Missing timing measurements
remain Video qualification work.

One Board attachment supplies the earliest Video deadline and advances the
phase from Core's existing elapsed ticks. A VBlank edge sets the existing
pending IRQ latch; acknowledgement clears it. A pending latch coalesces edges
until acknowledgement. Core owns all execution/time advancement and splits
at deadlines. No host timer, second scheduler, frame-count clock, COP response
or Shared change is introduced. The nominal 16 MHz time axis is marked
macro-proportional; the existing V30 L2 aggregate is not upgraded to verified
physical time.

Modulo reduction precedes phase addition, so long tick inputs do not overflow
the raster accumulator. Video returns whether any edge was crossed; Core's
deadline splitting preserves ordered real receiver delivery. Video rendering,
register semantics, DMA and COP operation models remain open.

## Verification And Review

The existing Board test owns both new proofs: private Video phase checks
immediately before/on the first edge and irregular slices, and an actual
configured V30 Board run. The latter installs a synthetic ISR, enters HALT,
uses the public next-deadline operation, receives and acknowledges three
periodic IRQs, and verifies ISR count, period, nominal-versus-physical metadata
and reset while another IRQ is pending. It does not access Core private
headers or add a public testing seam. CPU instruction and generic scheduler
behavior retain their existing unit owners.

An initial test build tried the Core-private arbitrary-time operation and was
rejected by strict compilation. It was replaced with the real public deadline
route; no Core header, declaration or dependency rule was weakened. Subsequent
focused Board tests passed on x64 and x86. Complete qualification results and
cleanup are recorded below before delivery.

Actual build review also found the local CTest executable missing from
build-unit-tests. Three product-local CMake lines now bind it to that existing
aggregate. The final logs explicitly show compilation and linking of the
latest Board test before CTest. Earlier passes using the older test binary
are superseded, not counted as final qualification. A same-issue read-only
sweep finds the MyNES aggregate mismatch and records it in this product's
TODO for the Shared tooling owner; S8 does not change that product.

| Run | Result | Log SHA-256 |
| --- | --- | --- |
| Initial x64 sandbox run | 686/687; repository.build-cleanup denied junction creation, Board passed. | `073E77516E1AD6CEAB3E97DD62E66017090BA6ED31992CA167C4CC546D2946C5` |
| Intermediate x64 | 687/687; superseded test binary before aggregate correction. | `89D4A549408C0617B14D7C795424576022C80858BAC10E04C2B272A2DF5B25AA` |
| Intermediate x86 | 687/687; superseded test binary before aggregate correction. | `E8547C921961F65DBD8456703D6B64D00CFE37BBDA39632F4646BCA9AEEE9DF6` |
| Final x64 | 687/687, zero failures/skips, 128.82 seconds. | `8294DA903FD01A17327BC250D95FA58F6519139684B88863673EA019DA0A67D7` |
| Final x86 | 687/687, zero failures/skips, 125.44 seconds. | `9C961EDB5899556714F0D16A7707894A7D0BF98BF666BA95483F8C87E7EECE3D` |

All full runs use the stable Ninja/ccache graphs, Ninja -j8 and four CTest
workers. The final runs execute after the local dependency repair; subsequent
Board target builds report no work on either width. No assertion, timeout,
test name or test count was relaxed. Documentation governance and whitespace
checks pass. No desktop, RDP, owner-ROM startup, full external integration or
playable game qualification was executed or claimed.

## Actual Delta And Retained Owners

Counted source/test paths are git diff --numstat for existing files plus the
29-line video.c and 16-line video.h. Production including local CMake is
+71/-2/net69; tests including local CMake are +112/-1/net111. The positive
test delta supplies the real ISR/reset receiver rather than another test file
or framework. New code consists of three private Video operations and two
Board attachment callbacks. One embedded raster field is added. There is no
new public API, allocation, thread, queue, clock axis or manager.

The one retained production route is Core timeline -> Board attachment ->
Video phase -> existing Board IRQ latch -> Core interrupt acknowledgement.
The existing explicit Board pulse remains a distinct external Video/device
input, using the same latch and acknowledgement rather than a second IRQ
path. COP deliberately retains its explicit unmodelled-command failure;
this batch does not imply that VBlank resolves that independent gap.

The six-file temporary root totals 775,694 bytes: one pinned research file
and five test logs. Their necessary identities/results are retained above.
Every launched build/test session is terminal before governed preview/apply
cleanup. Preview and apply removed exactly these six files and 775,694 bytes;
the stable caches remain. Product EXEs do not require refreshing: this Core has no admitted
MyRaidenII Product executable and no other App links this product library.
Shared components, other products, INIs, external ROMs and media are untouched.
