# M5 T580 S64 — PC110 8042/TrackPoint reset receiver

## Source boundary

`O:/repos.external/Open-Source-PC110/Discovery/Trackpoint/readme.md`, sections
5 and 7a, records a live PC110 controller command byte of `65h`: keyboard IRQ,
the system flag and translation are set; AUX is disabled and IRQ12 is masked.
The same record observes the attached U75 as a standard PS/2 mouse: `F2h`
returns `FAh 00h`, `E9h` returns `FAh 00h 02h 64h`, and `F4h` enables reports.
This is external live-system evidence (L2), not a recovery of the U75 mask ROM
or a general 8042 silicon specification.

## Receiver

`x86-nxvm` already owned one reset configuration family for board-selected
8042 electrical state. S64 adds its optional command-byte member. An
unconfigured controller still computes its existing portable AT default. A
configured controller stores one byte and applies it during the ordinary
controller reset; the same reset derives AUX admission from bit 5, so readback
and report acceptance cannot diverge.

MyPC110 selects `65h`. It continues to use the existing `A8h`, `D4h` and
standard PS/2 path; no TrackPoint MCU model, special input queue, mouse
firmware, ROM data or alternate host route exists.

## Direct proof

The pre-existing controller contract proves the generic unconfigured command
byte remains `43h`; its added isolated fixture proves a configured `65h` reset
suppresses reports until `A8h` enables AUX. The existing MyPC110 base
composition test uses public ports to prove the reset byte, rejected report,
then `A8h`, `F2h`, `E9h`, `F4h`, and the resulting packet.

This does not qualify actual TrackPoint cadence, physical motion calibration,
panel behavior, BIOS consumption, configuration persistence, guest boot or
PC110 artifact publication.

## Verification

Focused controller and MyPC110 public-port tests pass on x64 and x86.  The
complete configured repository test run passes 754 of 755 tests on each host
width, including all 714 `unit` tests and both MyPC110 real-firmware FDC-reset
and boot-handoff integrations.  The sole non-passing test is the existing
`lib.integration/native_midi_output`: this host has no native MIDI output
endpoint, so its setup assertion cannot create an output.  It is external to
the 8042 path and is not represented as a PC110 result.

The x86-nxvm and test/x86-nxvm manifests verify after the changes.  All four
existing PC products were rebuilt as stripped Release x64/x86 artifacts at
their current 0.5.0580 identity; MyPC110's four preview executables were also
rebuilt in the Ninja trees but remain unpublished, as this receiver does not
qualify a PC110 product release.
