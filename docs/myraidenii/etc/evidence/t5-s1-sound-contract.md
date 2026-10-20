# M1 T5 S1 — Seibu Sound Contract

## Scope

This record freezes the first sound implementation boundary. It uses the
existing MyRaidenII Board route and externally inspected behavior as L2; no
reference source, audio ROM, sample, PAL or generated output is copied into
the repository.

## Existing Local Ownership

`myraidenii_core_board` already owns the main-V30 side of the Seibu handoff:

- bytes `0x700--0x701`: main-to-sound command;
- bytes `0x702--0x703`: sound-to-main response;
- write `0x704`: sound interrupt request;
- read `0x705`: command-pending state;
- write `0x706`: command-pending state.

The public Board operations expose the same limited handoff to a future local
sound owner. They do not create a sound CPU or a second scheduler. Board's
existing attachment callback is the sole guest-time owner, so sound advances
only from that callback's elapsed main-board ticks.

## L2 Board Model

Inspected reference: `O:/repos.external/mame-raiden2-l2/raiden2.cpp`, Raiden
II sound map and configuration (current local research checkout). It supplies
these finite L2 facts:

| Fact | Local owner | Model grade |
| --- | --- | --- |
| Z80 program map has ROM `0000--1fff`, RAM `2000--27ff`, banked ROM `8000--ffff` | Core Sound CPU/map | L2 |
| Seibu control/latch/IRQ registers occupy `4000--401b` | Core Sound transport | L2 |
| YM2151 register pair is `4008--4009` and raises the Seibu IRQ route | Core YM/transport | L2 |
| Main command pair is read at `4010--4011`; pending and coin input are `4012--4013` | Core Sound transport | L2 |
| Response pair is written at `4018--4019`; bank control is `401a` | Core Sound transport | L2 |
| Two MSM6295 command/status ports are `6000` and `6002` | Core OKI devices | L2 |
| Z80 and YM2151 both use 28.636363 MHz / 8; each OKI uses 28.636363 MHz / 28, PIN7 high | Core clock conversion | L2 |

The Zilog Z80 manual, Yamaha YM2151 data sheet and OKI voice-synthesis
databook recorded in `etc/research/references.md` remain chip-level primary
sources. They define each device contract; they do not prove the selected
board's exact integration, hence the integration facts stay L2.

## Ownership And S Sequence

1. **S2 — transport and deterministic clock seam.** Add one private Core
   Sound owner attached to Board's existing elapsed-tick callback. It consumes
   the existing command/interrupt operations and exposes no host PCM or public
   sound-device pointer. Synthetic tests cover command/response, pending,
   IRQ acknowledgement, reset and rational tick advancement.
2. **S3 — Z80 execution/map.** Add an independently written local Z80 CPU and
   its fixed map/bank/RAM route. It is Core-local, driven only by the S2 clock
   seam, with instruction/timing cases derived from the Zilog manual. No x86
   Core or host thread is reused as a false second CPU abstraction.
3. **S4 — chip command semantics.** Add local YM2151 and MSM6295 register,
   timer/status and sample-address/channel behavior. Unit vectors come from
   the primary data sheets where available; unproven board integration stays
   explicitly L2.
4. **S5 — deterministic mix and receiving proof.** Mix accepted chip samples
   into one Core PCM stream under Board ticks, then qualify command-triggered
   output without a host audio worker. Emulator/Lib publication belongs to
   the later Product/Machine task.

This order keeps Board as the only composition root, Sound as the only
secondary-CPU/device owner and Emulator/Lib as the future host-delivery owner.
It deliberately does not invent a reusable arcade-chip framework for MyNeoGeo.

## Stops

An incompatible source license, need for a Shared public contract, need for a
second guest-time owner, or a requirement to retain protected source/data
blocks the affected receiver only. It does not justify a silent audio stub or
another execution path.
