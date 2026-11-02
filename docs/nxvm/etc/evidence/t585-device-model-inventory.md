# M5 T585 S1 — PC110 device and model inventory

This inventory is against T584 closure `ee68511b2` and the current source,
not the historical status in the T580 ledger. A boot or successful test of
one route does not qualify a whole physical device.

| Area | Current owner and positive evidence | Remaining work and boundary |
| --- | --- | --- |
| Firmware / variants | `app-mypc110/pc110_firmware.c`, selectable firmware and T583/T584 real product boots on both widths and variants. | Derived DSTN is not a verified original. A deliberate boot-path patch bypasses an unmodeled power-on suspend condition; it does not implement a complete power controller. |
| PC Card | `x86-nxvm/chips/rb5c396` owns two banks and socket input levels; App routes 3E0h/3E1h. Chip and App unit tests exercise registers. | App supplies no insertion input; windows, card mapping, change interrupt and media are not implemented. Onboard ATA is a separate path. S2 must research and implement a bounded, tested controller/board route rather than claim that empty-slot tests cover cards. |
| Internal storage | App composes the existing ATA PIO controller with two slots; T583/T584 actually booted DOS/Personaware on private media and preserved original files. | This proves the tested boot configuration, not PC Card storage, all commands, flash programming or dock floppy. |
| Power | Chipset suspend-refresh and F12 firmware resume are wired. T580 S71 records an actual firmware suspend/resume checkpoint. `pc110_ec_block_interface.h` explicitly describes a register file, not an EC program or live power model. | Battery, adapter, power-off, guest APM and general suspend/wake contracts are not qualified. Determine evidenced guest-visible behavior and independently test it; do not equate Emulator pause with guest suspend. |
| Audio and auxiliary | Existing board PIT/speaker and LPT routes are not proof of ES488 audio; App has no ES488 or OPL binding. | Audit reference behavior and implement the necessary PC110 audio route with deterministic guest/device evidence; inventory serial, IR, modem and dock separately. Do not claim audio from a host mute test or a silent placeholder. |
| Display and font | App wires F65535 and external font ROM; T583/T584 real firmware rendered Setup, DOS and Personaware in both variants. | Physical panel timing, FRC/dither and color equivalence are not established by screenshots; derived DSTN provenance remains explicit. Verify software-visible mode transitions and cursor separately from physical-panel claims. |
| Input | Keyboard, TrackPoint and distinct inking path are wired; T584 exercised guest calibration, CMOS persistence and keyboard navigation. | Posted key messages and Machine-queued pointer events do not prove physical capture, physical chords or RDP, retained for final qualification. |
| Memory | `pc110_plan.c` explicitly selects 20 MiB and rejects other explicit sizes. | Additional configurations remain separately recorded in NXVM TODO; no unsupported RAM size is silently rounded. |

The selected follow-up sequence starts with S2 PC Card status/register
semantics and bounds, S3 status-change interrupt routing, S4 card memory-window
decoding, S5 the Core/PC110 memory route and S6 card I/O/media and remaining
status inputs. S7 addresses power/APM and the controller's INTR#/RI_OUT# wake
routing, S8 audio and auxiliary devices, S9 display/font and remaining-model
reconciliation, and S10 complete device coverage and qualification. Split a
batch into further numeric S tasks if
implementation or verification warrants it. These names are planning scopes,
not assertions that one S can implement every feature. Original manual facts
alone are L3; reference-derived models are L2; unsupported estimates are L1.

Reference evidence: [T558 receiver checklist](t558-s2-receiver-checklists.md),
[T580 PCIC](t580-s58-pcic-receiver.md), [T580 power](t580-s71-postsetup-power.md),
[T583 first setup](t583-first-setup-ledger.md), and
[T584 interaction](t584-product-interaction-ledger.md). Preserve their
historical states and use the current source for present implementation.

Reference comparison: local PC110-QEMU `bae390ebf21665ec97860d28e26ec46f1fe8f4e9`
and PC110-EMU `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c` both leave the
3E0h PC Card route as an empty placeholder; neither establishes populated-card
window behavior. Open-Source-PC110 `02ccf2e8e19b298a2cf04c5d430278eb087cc368`
identifies the physical controller and audio wiring. These are references,
not imported implementations.

S1 verification: complete cached x64 repository unit suite 712/712 passed in
121.43 seconds; log SHA-256
`3fd33932e132e8338b3ac640bee13c6464d5df38d1ac33d3888a573729d3bc1d`.
Documentation gate and `git diff --check` passed. This static inventory did
not itself execute every device or prove full product qualification.
