# T585 S16 — ES488 playback delivery

The existing board creates `shared_dma` and PIC endpoints before invoking the
profile factory; topology configuration subsequently binds the FDC channel and
refresh. Therefore the profile can be given a borrowed DMA bus at construction
and bind channel 1 without constructing another controller. This corrects the
earlier S14 wording: *topology binding* follows the factory, but the DMA bus
itself already exists. Failed profile construction and finalization must not
leave a provider borrowing a released device. The 8237 `write_device` provider
receives memory-to-device data; requests use its existing binding nonce.

The board already asks its profile for a guest-time deadline, but does not yet
invoke it to advance a due device. A single bounded profile advance callback
on the existing Core time axis is a candidate; it must run in the correct
ordering relative to DMA and PIC. ES488 sample rate, DRQ cadence, single-cycle
completion and IRQ acknowledgement must be checked against the referenced
SB2-compatible behavior, while the configured DMA/IRQ default is explicitly a
model. No host wall clock may advance guest samples.

Core's scheduler advances arbitration before `before_internal_advance` at a
due tick. A DRQ raised by the profile's due callback therefore becomes
eligible for the following arbitration, rather than retroactively consuming a
cycle that has already completed. A regression must check this ordering and
the transition from a pending request to the next scheduler deadline, including
the HLT path. The profile's existing deadline callback alone does not perform
the device work.

The outer Machine currently has no PCM producer. Lib PCM accepts only an
ordered prefix and may report `LIMIT_EXCEEDED`; a producer must retain any
unaccepted suffix and must never block in a Core/device callback. The runner
has existing bounded quantum and Session rendezvous boundaries. Before changing
the contracts, resolve how the single worker drains samples at those boundaries,
how it yields for host backpressure without preventing stop/pause, how turbo
avoids overproducing a bounded queue, and how reset/teardown safely discard or
finish pending samples. The existing MyNES and MyRaidenII adapters demonstrate
Lib ownership and accepted-prefix handling but are not PC110 chip models.

The in-progress implementation binds channel 1 and IRQ 5 as explicit L1
defaults, implements the SB-compatible 40h sample time constant and 14h
single-cycle 8-bit DMA command, D0h/D4h pause/resume, D1h/D3h speaker gating,
and 22Eh interrupt acknowledgement. An existing Machine integration drives
the actual guest scheduler and checks the resulting PIC request; an existing
construction test checks actual 8237 memory transfers, pause and ordered sample
bytes. Neither test proves general ES488 command coverage or physical timing.

The current App adapter performs zero-order-hold conversion onto the existing
48 kHz Lib stream, retains partially accepted prefixes, and waits only at an
executor boundary using Lib's cancellable wait. An isolated fake-stream test
checks conversion and prefix retention; it is not a native-audio test. With no
host endpoint, guest DMA remains functional and generated samples are discarded.
If the bounded sample queue fills, the L1 model withholds the next DMA request
until the executor drains it; this is host backpressure, not a hardware-exact
overrun model. The real product-composition test observes IRQ5 after guest
execution and after a `HLT` wakes into its interrupt handler. Native PCM
submission is separately checked through Lib's accepted/submitted counters on
both host widths; that is not a physical audibility claim. YM3812/FM is not implemented by S16.

The first real DMA route exposed a separate page-address defect: the PC110
sixteen-byte, independently readable 80h--8Fh bank was not consulted by DMA,
so transfers above 64 KiB incorrectly read page zero. The later
Open-Source-PC110 `Discovery/Chipset/readme.md` reports live DMA page values;
it contradicts interpreting this bank as *only* passive scratch. The bank's
independent values and 8Bh firmware latch remain intact. The transfer path now
uses the conventional channel offsets as an L1 page-wiring model. Existing
Shared DMA tests exercise high pages with both one and two controllers and all
seven usable channels, and the PC110 transfer test uses a sample above 64 KiB.
This does not establish every physical DMA page wire or prove operation with
every PC110 peripheral.

## Qualification and limits

Cached x64 and x86 unit suites each passed 713/713. The x64 full suite
preceded the integration-only native submission instrumentation; afterward
15 affected checks passed. Both architectures subsequently passed all 48
registered integration tests in the isolated desktop. The first x64 run had
two failures from stale integration executables after the Shared interface
change; rebuilding the integration targets made those two pass, followed by the
complete successful rerun.

Each architecture also passed the fixed-profile boot and configuration checks:
My5160 3/3, My5170 5/5, DeskPro 386 6/6 and default NXVM 3/3. MyRaidenII's
separate bounded selected-ROM probe and deployed Console lifecycle check passed
on both architectures; this is not evidence that its game is playable.

The PC110 guest integration executes DMA, observes IRQ5 and HLT wake, and checks
that actual Lib PCM enqueue accepted both ordered guest samples. A controlled
fake-stream unit test covers partial submission, wait/cancel and close failure;
it does not prove audible sound or inject native cancellation/join failures.
Host audio is optional. All fourteen product artifacts were built or verified
against the current cached build; the two MyNES binaries were byte-identical
and the MyRaidenII differences were PE timestamp/checksum only. The ten PC
binaries and the two refreshed MyRaidenII artifacts are delivered separately
according to their target ownership. User ROMs, media and configuration remain
unchanged.

The model does not implement FM synthesis, all ES488 commands, physical DMA
page wiring, precise chip overrun timing or complete real-hardware sound.
Those remain for subsequent device work and qualification.
