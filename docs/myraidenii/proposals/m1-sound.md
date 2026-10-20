# Sound Qualification

## Objective And Boundary

Implement the selected Seibu sound subsystem: Z80 execution/banking/interrupts,
YM2151 synthesis and two OKI sample paths, connected to real board commands.
Lib PCM is host delivery, not these chips; Lib FM is not a software YM2151.

## Work And Verification

The accepted S1 ledger fixes the four bounded receivers:

1. Board-owned command/response transport and deterministic sound clock seam;
2. local Z80 execution, fixed address map and bank route;
3. local YM2151/MSM6295 register, timer/status and channel semantics;
4. deterministic mixing and command-triggered Core PCM proof.

The existing Board command/response/pending API is the one transport path.
Sound advances only through Board's existing guest-time callback. Keep
mechanisms under their real owner; no speculative shared chip framework for a
future Neo Geo consumer. Test CPU vectors, latches/IRQ/timers, sample
addressing, mixing and deterministic PCM independently of physical volume.
Then observe real attract/gameplay/music/effect command paths and PCM delivery.

## Exit And Stops

The finite sound ledger passes in both widths, including partial enqueue,
flush/stop/reset and cleanup. Guest time remains independent of host audio
worker scheduling. No silent success stub or substitute music; source imports
and Shared changes require separate approval, not an assumed license exception.
L2 board integration remains explicitly labeled; it is sufficient to implement
the bounded local model, never a claim of physical-silicon accuracy.
