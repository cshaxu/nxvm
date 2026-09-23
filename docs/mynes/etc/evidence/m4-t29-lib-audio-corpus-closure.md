# M4 T29 Lib Audio Corpus Closure

## Outcome

T29 makes MyNes Lib Audio a canonical shared-corpus candidate. The dead
platform-query endpoint is gone, FIFO capacity and playback batch sizing are
private `src/lib/audio` policy, and finite PCM has a public delivery boundary.
No SoftPC source changed.

## PCM Delivery Contract

`lib_audio_stream_enqueue` still copies ordered PCM without blocking. Full
private batches proceed asynchronously. `lib_audio_stream_flush` drains every
accepted frame, including a final sub-batch tail, and returns after native
acceptance or a terminal error. Deactivation performs that flush before it
clears unsounded native PCM. `clear` intentionally discards queued PCM.

Destruction completes a sole queued tail while it is safe to do so. If a full
batch is blocked in the native backend, destruction cancels that wait and tears
down without another native write. A finite producer that requires every
accepted frame to reach the backend must call `flush` before destruction.
This preserves the existing worker-before-platform-destroy lifecycle rule.

## Verification Record

| Check | Result |
| --- | --- |
| Production query and public-policy sweep | Passed: no `audio_stream_platform_query`, `LIB_AUDIO_STREAM_PLAY_BATCH`, or `LIB_AUDIO_STREAM_QUEUE_CAPACITY` remains in `src` or `test`. |
| Deterministic Audio contract tests | Passed: explicit flush, deactivation, clear, sole-tail destruction, terminal failure, and blocked-native-wait destruction. |
| Lib corpus integrity and dependency checks | Passed on x64 and x86: manifest, test manifest, and component DAG. |
| Complete x64 build and CTest | Passed: 112/112. |
| Complete x86 build and CTest | Passed: 112/112. |
| Tracked release artifacts | Rebuilt and present beside the sole editable `assets/binary/mynes.ini`. |
| Remote delivery | No remote is configured; local commits satisfy the approved remote-aware rule. |

## Artifact Hashes

| Artifact | SHA-256 |
| --- | --- |
| mynes_0_1_0011_x64.exe | B6217839EEE0037C9F14700C739A568E7D6C24B5F7E2EDB0F60D1C951C369CAE |
| mynes_0_1_0011_x86.exe | 944DDC62B1040D453579D5D1A1D89DA18BC199E490D0DB54C46F2C6E62C571C1 |

## Shared-Import Boundary

The Lib implementation uses only Lib Types and Lib Base, and its scheduling
constants stay private. A consumer with a finite sound (for example an x86
beep adapter) uses PCM enqueue followed by `flush`; it does not own waveOut
buffers, worker wakeups, or host playback timing. This is the remaining
consumer-side integration requirement for a verbatim shared Lib import.
