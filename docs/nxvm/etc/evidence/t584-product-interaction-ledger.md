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

## S3: guest inking calibration through Emulator Machine

The current Product factory and Emulator Machine were exercised on both host
widths for both display variants with guest-saved CMOS and private overlay
media. A task-owned observer submitted absolute KVM mouse contact/release
events through the existing Machine queue only after each guest screen was
observed. The actual DOS driver progressed through upper-left, upper-right,
lower-left and lower-right prompts, displayed that calibration data was saved,
and then reached the Personaware launcher. The four Machine probes exited 0.
These are actual guest-driver and Machine/IRQ observations, not full App Window
capture, physical mouse or RDP evidence. A guest save into an overlay is not
proof that calibration persists after the overlay is discarded.

The shared prompt frames, in order, have PPM SHA-256 values
`75a43c909f79609524cbd7a1c1b938ff39023ea28c116c275b52738e4b8f2b29`,
`d73ddfd8d9e847158a1371a1c0753087939d81ff6355cf40d98ea86f7e7e6932`,
`837aea1a909f3e502e2a0e660e6ee8409ded58f4a1cc24d9336051e9019eef58`, and
`d1b005bc3fc694de1557dacbc41e0fe064dd755c3b562da4e1fb1bdb7253dded`.
The saved-calibration text region and launcher region were verified separately
because cursor and clock pixels vary. A first matrix observer incorrectly
required the entire saved frame to match; after inspecting the actual saved
text, its task-owned watcher was corrected to match the stable text region.
No product code or guest state was altered by that observer correction.

Probe-log SHA-256 values are DSTN x64
`dc5423805ff55e9e23909fa2e035876056ee74f4a7296b0e7b3b235adf94fcb2`,
DSTN x86 `b204b849220332620e8497278fc38c10d00398acf4ec0f94589c7946573c822f`,
TFT x64 `17342073550865eaefe17b630a04fd2a2b0b39785801613846b712d4753a75b3`,
and TFT x86 `6681c20a5a123fc36c487fcbc110648ba0de219c40af6a2e4dca5445f1a15490`.
The two private source-disk hashes were unchanged in each case. The x64 full
repository unit suite passed 712/712; its log SHA-256 is
`bec5e1d45d55d56c0a72c6541fb64aed30c48a2329266382f5f4b1788412bcaa`.

## S2 in progress: observer correction and test contention

Both widths and variants reached the actual Personaware World Clock and the
monitor reported pause/resume. The TFT x64 and x86 observers subsequently
returned failure while waiting for final process exit; neither result is a
complete pass. Inspection of the paused-console snapshots exposed a flaw in
the task-owned observer: its second pause check could match the first
`Machine paused.` message even when the most recent notification was
`Machine resumed.`. Thus it could send `exit` before a fresh pause completed.
This proves the observer's readiness check was insufficient; the old log did
not establish whether that alone caused either failed exit. The observer is
being corrected to require a pause notification after the latest resume and
to log the native wait result and process exit code. This finding also limits
the interpretation of T583 S8's final pause acknowledgements; its separately
observed stop/start and reset/resume POST/DOS results remain intact.
Initial TFT x64/x86 observer logs have SHA-256 values respectively
`9ed03f46658a2da899361a70647144e4d51f51f30ad4033cb3ab2ccd0743b439` and
`de734bbd4c1695f221fbf0065419b17616674ea91c4681258e8fdba27312c33e`.
Both TFT cases were independently rerun through calibration escape, launcher,
World Clock, pause/resume, application return and normal exit using the revised
observer. Both recorded `WAIT_OBJECT_0` and process exit code zero. Their x64
and x86 observer logs have SHA-256 values respectively
`3b7849528dc133c83d7f8d8294c1b9de7a8180067dbe1baec5f4766a735836b2` and
`91a27bf0c880f39be079cf73c328624494bf833c0060b6d3e27a7523f9d054cc`.
The original DSTN x64 and x86 cases both reached the same product milestones
and exited zero; their observer log SHA-256 values are respectively
`524b190d12163635b63d2df02ff6a7ba413527e457f3afaadfa4a3e847cd7492` and
`1eef9608439262cf514df8e3c14a7e26963cf687e5968475ecf7e4889f0ce87f`.
All four accepted cases retained the original hashes of both task-private
input disk copies. These observations cover native keyboard messages and
product lifecycle, not real host mouse capture or interactive RDP.

The first x86 full unit run had 710 passes and two timeouts:
`lib.types-layout-selftest` (180 seconds) and `emulator.verifier-negative`
(60 seconds). A later serial focused rerun passed them in 103.81 and 31.53
seconds respectively. The host also had an independent SoftPC test process
running; concurrent activity is a possible contributor, not a proved unique
cause. The original failed run remains recorded, and a complete unit rerun is
was required before S2 closure. The complete serial x86 rerun then passed
712/712 with no timeout-budget change. SHA-256 values are: original failed
full run `af001d8d2bfe2a6200b697906c07207fcbe3684c5bfd69afe45ebecc3be86a9f`,
focused rerun `6b5888e01f30fe7e1c94ca2d7d372a36e148a71ca2fe23796851b55b6ff3dfa2`,
and full passing rerun
`72deab73b113dff2f0c7ffdb939724db116cbb1e4e6e010176f251b1436245e8`.
