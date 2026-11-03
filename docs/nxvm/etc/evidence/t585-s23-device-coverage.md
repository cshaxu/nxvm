# M5 T585 S23 — current PC110 device coverage

This ledger supersedes the then-current implementation statements in the S1
inventory. It is a reconciliation against source after S22, not a claim that
every physical PC110 peripheral has been reproduced. The first-time setup and
product interaction results belong to T583/T584; T586 owns final product
qualification against the current artifacts.

| Area | Implemented path and evidence | Explicit remaining boundary and owner |
| --- | --- | --- |
| Firmware and identity | Two selectable firmware variants and one x86/x64 product pair; T583/T584 exercised Setup, DOS and Personaware. | The DSTN image is derived, not a verified original dump. The App's bounded POST-suspend compatibility patch remains; native RDP and real-device equivalence belong to T586. |
| PC Card registers and interrupt | RB5C396 status, detect, IRQ, I/O signal and wake go through actual PC110/PIC routes; S2–S4 and S8–S11. | Card-detect pins are combined; electrical polarity and timing are not fully modeled. External I/O-card transport remains absent and is retained as peripheral debt. |
| PC Card media | Both socket memory windows use the Core's dynamic range and Storage leases; S5/S6/S10 include guest-bus and product lifecycle checks. | No physical-card compatibility claim; programmable waits are decoded without a cycle-accurate timing claim. I/O-window decoding is not an emulated I/O-card receiver. |
| Internal disk and floppy | Internal ATA is a separate board path, with real DOS boot evidence. S49 established the floppy controller reset/IRQ route. | A dock or real external floppy device is not established. S13 explicitly corrects the earlier suggestion to route internal ATA through the PC Card controller. |
| Power and APM | Guest chipset suspend/F12/RSM has a real-firmware checkpoint; S22 exercises installation, connection and status via real BIOS INT 15h for both firmware variants and host widths. | Default AC-present state reports battery unknown. Dynamic AC/battery/charging/lid models are not supplied; these are retained as peripheral debt, not host Emulator pause. |
| Audio | ES488 single and auto-init 8-bit DMA, IRQ and speaker controls plus OPL2 register/timer and bounded FM mixing use the existing guest clock and PCM stream; S15–S19. | FM is L1 melodic synthesis without rhythm, LFO, key scaling or exact tables. Complete ESS modes and live physical listening remain unqualified. |
| Serial | S21 adds a reusable 16550-compatible chip, board-selected clock, COM1 decode and IRQ4; real guest instructions verify loopback and PIC delivery. | No host serial RX/TX endpoint is connected. IrDA, modem and dock serial are not implied by internal loopback. |
| Display and input | F65535/font, keyboard, TrackPoint and inking paths are wired. T583/T584 cover firmware, guest and injected/native-message observations; S21 tests real cold and resume firmware paths. | Physical panel color/FRC, raw host mouse capture, physical chords and interactive RDP remain final-qualification or later fidelity items. |
| Configuration and memory | One product selects DSTN/TFT through the stopped-state command; both widths are released. PC110 uses a fixed 20 MiB layout. | Additional RAM configurations and startup performance remain separately recorded in TODO. |

S21 qualified both cached unit suites 714/714 and registered integrations
47/48 on each width. The sole failure was native MIDI output creation in the
unavailable isolated-desktop audio environment; the owner explicitly allowed
this environmental exception. Fourteen release artifacts were built and PE
checked; XT/AT/DeskPro selected-profile integrations passed on both widths.
S22 changed only the existing TFT cold-boot test and passed it on both widths;
temporary probes also exercised the derived DSTN firmware on both widths.
These results do not establish live host serial, audio audibility, RDP, or
complete device fidelity.

Actual source/test line counts from `git show --numstat`, excluding
documentation, manifests, CMake and binaries: S21 Shared adds 252 C/header
lines and 75 test lines; the PC110 receiver adds 126 C/header lines, removes 3,
and its tests add 74 lines and remove 1. The increase implements one new chip
and its board binding; it does not add a second clock or execution path.
S19 adds 40 production lines and removes 11, with 40 test additions and 4
deletions for real DMA blocks and guest playback. S22 adds 69 lines to the
existing firmware test and changes no production source. Those counts do not
characterize the full T585 diff.
