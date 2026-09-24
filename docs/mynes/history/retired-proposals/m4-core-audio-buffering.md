# M4 Core Audio Buffering

## Outcome

Move MyNes-specific PCM accumulation, fixed submission size and pause/reset
discard behavior from the retired local Lib design into Core. SoftPC Lib remains
the unchanged generic i16 PCM submission interface.

## Acceptance

Core never submits a partial live-playback block, preserves FIFO sample order
under Lib backpressure, clears staged audio on reset or deactivation, and emits
silence as zero PCM frames. A missing native audio endpoint is a degraded host
sink, not a machine-construction failure, so Window/Console input remains usable
in RDP sessions.

The Core controller adapter consumes KVM's normalized virtual-key identity:
WASD is D-pad, J or either Ctrl is B, K or either Alt is A, Enter is Start, and
either Shift is Select. It intentionally does not require a scan code, because
physical keyboards, RDP and virtual keyboards do not all report one. Lib/Common
retain their original all-key normalization and presentation-input contracts.
x64/x86 audio, Core, KVM input and game-path regressions pass.
