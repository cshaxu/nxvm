# M1 T5 S10 Machine PCM Bridge

## Boundary

`myraidenii_core_board_pcm_copy()` remains the only Core-to-Machine PCM
operation. `myraidenii_machine_audio` is the only Machine adapter in this
increment: it borrows the Board and a Lib PCM stream, stages copied frames,
and submits ordered prefixes through `lib_audio_pcm_stream_enqueue()`.

The adapter has no guest clock, worker, callback, mixer, frame route, Product
reference or host-platform implementation. Lib remains the sole native queue
and playback owner. The later Machine driver owns stream creation, lifecycle,
input, frame publication and its binding to Emulator.

## Accepted-Prefix Contract

The stream's accepted count is authoritative. The bridge removes exactly that
prefix from its FIFO. A zero or partial result returns the existing status (or
`LIMIT_EXCEEDED` for an otherwise-successful short acceptance) and leaves the
remaining suffix staged for the next call. `clear()` calls Lib first and only
discards local staging after that existing operation succeeds.

Core reports its own already-dropped source frames separately. The bridge
returns `LIMIT_EXCEEDED` for that discontinuity; it neither invents a replay
nor claims lossless host delivery after the Core queue has already overflowed.

## Evidence Grades

| Fact | Grade | Basis |
| --- | --- | --- |
| PCM stream accepted-prefix ownership and clear semantics | Existing Lib contract | `lib_audio_pcm_stream_interface.h`; local repository contract, not a hardware-fidelity claim. |
| Board produces 48 kHz interleaved stereo PCM | Existing Core contract | Now named once in `core/audio_interface.h`; this preserves the existing local Sound format. |
| Staging FIFO capacity and retry behavior | L1 | Local bounded engineering mechanism. It has no manual or external-emulator fidelity claim. |

No manual PDF formula/constant is adopted by this S, so it makes no L3 claim.
No external emulator behavior is adopted, so it makes no L2 claim.

## Regression

`test/myraidenii/unit/machine/audio_smoke.c` compiles the actual production
bridge with only fake lower-boundary functions. It proves a first submission
that accepts one of three frames leaves the other two for the second drain,
with the final submission exactly equal to the original ordered source. It
also proves a failed Lib clear retains the staged source suffix. The fake is a
test-owned boundary substitute, not a production route.

The S's dual-width build and test commands/results are recorded in the closing
Current summary. Native endpoint, desktop, audible-output and external-ROM
qualification are intentionally not inferred from this regression.
