# M4 RP2A03 Audio Hardware

## Outcome

Implement deterministic five-channel RP2A03 audio and bounded PCM generation
from the existing machine-owned clock.

## Acceptance

Project-owned CPU/MMIO fixtures prove pulse, triangle, noise, DMC, frame
sequencer/status IRQ and DMC arbitration. Sample generation is continuous and
bounded, but native playback and host pacing remain separate receivers.
