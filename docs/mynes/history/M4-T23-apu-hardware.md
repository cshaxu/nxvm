# M4 T23: RP2A03 Audio Hardware

T23 supplies the machine-owned RP2A03 audio path required by both admitted ROM
profiles. Core now owns pulse 1/2, triangle, noise and DMC state; CPU bus
access to $4000--$4017; frame/status IRQ; DMC bus requests; and a bounded,
copied 48 kHz PCM FIFO. The APU is clocked once per CPU bus slot, while PPU
still advances three dots in that slot. Pulse timers run on alternate CPU
cycles; triangle/noise/DMC use their device periods. Frame-quarter/half events
drive envelope, length, sweep and triangle-linear state.

DMC reads are requested by the APU and completed only by Machine's production
arbitration path before OAM DMA/CPU work. The bus also returns open-bus data for
reads from APU write-only registers, which fixed the real indexed-store dummy
read observed at $4000 in the owner-local Dr. Mario run.

`mynes.core.apu-smoke` covers MMIO, waveform variation, bounded PCM production,
frame and DMC IRQs, DMC request/read completion, envelopes, sweep and triangle
linear state. `mynes.core.owner-rom-probe` runs ignored owner-local inputs only
when environment paths are supplied. On x64 and x86, both Dr. Mario/MMC1 and
Super Mario Bros. 2/MMC3 reached 20,000 driver slices (about five million
instructions), produced 534/526 frames respectively, and had no Core trap.
The PCM FIFO intentionally saturated because T23 has no host consumer.

The complete Core suite passes 33/33 on x64 and x86. T23 makes no native sound,
host pacing, full analog-accuracy or game-playability claim. Those outcomes
transfer to T24's reviewed upstream generic audio stream and the final M4
acceptance task. Lib/Common source remains unchanged in MyNes.
