# T584 MyPC110 product interaction ledger

T584 is the owner-directed second PC110 receiver. T583 establishes Setup,
DOS, separate-process configuration retention and in-process restart on both
host widths and both firmware variants; its native Window messages were sent
on isolated desktops and are not physical input or RDP qualification. One
DSTN x86 observer exit check returned failure without a discriminating reason;
an independent full repeat exited zero. See the [T583 ledger](t583-first-setup-ledger.md).

## Current input and presentation ownership

`app-mypc110/pc110_plan.c` consumes F12 for the firmware resume button and
maps host mouse events to the inking controller while passing those events on
for the independent PS/2 pointing device. `x86-nxvm/machine` handles the
ordinary keyboard and PS/2 path. `lib/kvm-window/win32` owns native keyboard,
raw mouse capture and the first-click capture gesture. `emulator/ui` owns
presentation and the Console status surface when a graphical Window is in use;
the Console component does not render graphical pixels.

Existing `mypc110.unit/inking` and controller tests prove bounded local
contracts; the unregistered real-firmware `--setup-pair-os-inking` diagnostic
can inject four inking corners into the Machine driver. Neither is an actual
native Window capture or human RDP pointer test. T583's product tests show
native key messages, setup, DOS and lifecycle; they had not established actual
Personaware interaction beyond its calibration prompt.

## S1 actual product keyboard observation

The current released x64 EXE, derived DSTN firmware and guest-saved CMOS ran
on a separate invisible Windows desktop with private copied media. The actual
Window reached the DOS prompt asking for the upper-left inking-pad point. One
native Enter left that prompt visible. Native Escape then left calibration and
the actual product reached the Personaware launcher; its observed BMP SHA-256
was `59ebd710212b73e1ff4f1a64b3836a3845d0ea5a3408ec3b0a35c89729965dc6`.
Native Enter on the selected World Clock opened the real application, showing
`1995-01-01 12:00`; its observed BMP SHA-256 was
`ed66a54d35eca7b9ed47894ea4652950031bdb92911d1b4754ea078e1d23d9d5`.
Escape produced the application's exit confirmation, and Enter returned to
the launcher. The process paused and exited zero normally; observer log
SHA-256 is `ba211916d9d3ec32291a48b1524a66d6586e6211138e7407606678460f52b3b7`.
Both private input disk hashes remained unchanged. This establishes guest
response to native messages in one real product configuration, not actual
interactive RDP input, four-corner inking calibration or pointer capture.

The x64 complete repository unit suite passed 712/712; log SHA-256 is
`dbdc44dc11c4f8a768522cf499e9f4e3df54a9d07d594e04881510c78d374c4b`.
Host session enumeration showed an active local Console and an RDP listener,
but no active RDP session. No user desktop was switched or foregrounded.

## Remaining receivers

Repeat actual Personaware keyboard entry on the other variant/host combinations;
exercise pause/resume in the application and observe teardown. The first-click
native capture, raw pointer stream, four-corner inking calibration and live
Console/RDP paths remain unverified by S1. Diagnose the earlier exit-check
uncertainty without inferring a product bug. Do not use a Machine-injected
pointer event as evidence that native Window capture or RDP input worked.
