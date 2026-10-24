# T571 S1 — PC110 Audio And Auxiliary Source Gate

## Scope and method

This audit compares MyPC110 and reusable X86-NXVM mechanisms with the
read-only Open-Source-PC110, PC110-EMU and PC110-QEMU research trees. It
imports no third-party source or binary and adds no audio, serial or dock
route.

## Disposition ledger

| Candidate | Source-qualified facts | Current owner | S1 disposition |
| --- | --- | --- | --- |
| ES488 Sound Blaster control face | Open-Source-PC110 live hardware probe establishes `220h` base, reset write `1` then `0` at `226h`, `AAh` reply at `22Ah`, `E1h` command returning DSP 2.01, and ES488 identification. The board reconstruction establishes bespoke Pluto/Bowman AEN, DMA and reset signals. | No current MyPC110 audio route. | The control observations are retained as future L2 input, but no chip is admitted now: command FIFO, rate, DMA channel/IRQ selection, completion/error/reset timing and PCM ownership are still incomplete. |
| YM3812 OPL2 | Live hardware probe establishes non-`FFh` status at `388h`; board reconstruction identifies discrete YM3812/YM3014 signal chain. | No current OPL owner. | Not admitted as audible synthesis. Port response alone does not establish clock, timer/IRQ, complete register behavior, sample generation or connection to one Lib Audio stream. |
| PC speaker | The physical board contains a speaker amplifier and normal PC speaker wiring is a useful reference behavior. | Existing PC/AT speaker wiring is not a PC110 composition. | Not admitted as a substitute for ES488/OPL audio. Speaker-only output would conceal the required chipset, mixer and DMA contracts. |
| FDC37C665IR serial/parallel/IR | Open-Source-PC110 identifies the Super-I/O and reconstructs physical serial/parallel/IR routing. | No PC110 Super-I/O configuration/decode owner. | Not admitted. Required configuration unlock/defaults, enabled logical-device decode, FIFO/IRQ/baud/handshake, reset and dock/IR power semantics remain unqualified. |
| Modem and dock | Pluto/Bowman documentation identifies gating and physical connection. | No CPU-visible endpoint exists in MyPC110. | Not admitted. Physical net ownership is not a modem command, dock-presence or power/error protocol. |
| PC110-EMU/QEMU peripheral paths | PC110-EMU explicitly labels ESS, OPL, LPT and related handlers as placeholders; PC110-QEMU publishes PC-speaker output while its PC110-specific peripheral windows are `FFh` stubs. | External L2 research only. | Rejected as production behavior. Neither provides a faithful ES488/OPL/Super-I/O contract. |

## Boundary review

`lib/audio` is a host PCM service, not a guest audio device. It must receive
one device-owned stream after that device has a deterministic PCM, DMA and IRQ
contract. Connecting a host speaker directly to `220h` or `388h`, creating a
second guest clock, or treating host audibility as a test would violate the
existing ownership boundary.

The existing PC/AT speaker and Super-I/O-adjacent paths likewise cannot be
attached merely because the PC110 shares familiar connectors. PC110 has bespoke
Pluto/Bowman decode, DMA and power gates. Reusing an independent mechanism is
possible only after its PC110 wiring contract is known.

## Result and transfer

S1 admits no production code. Its positive result is a precise, bounded
future-control probe: ES488 SB2 reset/version at `220h` and OPL2 status at
`388h`. That is insufficient to claim playback or to implement a fake success
path.

The real-firmware POST/setup/guest-boot receiver must capture `PS2.EXE`'s
audio configuration, including `ADDAUdio`, `IRQAudio` and `DMAAudio`, then
observe DSP command, DMA, IRQ and failure/reset transactions. It also owns the
Super-I/O logical-device configuration and any actual serial/parallel/IR/dock
use. A later receiver may then select one App-local audio/device owner and one
stream path without modifying the public host audio API.

