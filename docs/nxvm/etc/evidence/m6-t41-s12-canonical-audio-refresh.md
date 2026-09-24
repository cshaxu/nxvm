# M6 T41 S12 Canonical Audio Refresh

## Scope and provenance

The owner admitted one Shared audio slice from clean project-owned MIT SoftPC
commit `df9d1cf8`. Shared P1 is `4faf19eb7`
(`Shared M6 T41 S12 P1: import canonical WASAPI audio`). It imports only the
canonical stream implementation, its Windows Types boundary, the two admitted
audio tests, and their Lib build/manifest ownership. No App, firmware, media,
asset, executable, or public `lib_audio_stream` interface change is included.

The following files are byte-identical to that SoftPC revision:

- `src/lib/audio/stream_interface.h`
- `src/lib/audio/stream.c`
- `src/lib/audio/stream.h`
- `src/lib/audio/linux/stream.c`
- `src/lib/audio/win32/stream.c`
- `src/lib/types/win32/audio.h`
- `test/lib/audio_stream_smoke.c`
- `test/lib/audio_native_smoke.c`

P1 changes 467 insertions and 391 deletions. It removes the obsolete
`audio_win32_platform_smoke.c` white-box `waveOut` test, replaces the Windows
four-slot `waveOut` implementation with one worker-attached WASAPI shared
endpoint driven by readiness events, and retains Linux as an explicit
`UNSUPPORTED` platform leaf.

## Type and ownership audit

`src/lib/types/win32/audio.h` is the sole Shared production/test location that
includes Windows audio SDK headers or names `IMMDevice`, `IAudioClient`,
`IAudioRenderClient`, `IAudioCaptureClient`, `WAVEFORMATEX`, `HRESULT`, or
`GUID`. It publishes only Lib-prefixed aliases to the Windows leaf; public
`stream_interface.h` remains neutral and exact-parity.

The post-import sweep of `src/lib` and `test/lib` for `waveOut`, `WAVEHDR`,
`HWAVEOUT`, `mmsystem.h`, and `winmm` has no hits. `src/lib` and `test/lib`
manifest verification and the Lib dependency verifier pass.

## Verification

- x64: `library.audio_stream` and `library.audio_native` pass; the complete
  repository-only suite passes **335/335**.
- x86: both audio tests compile and pass against the i686 MinGW target; the
  complete repository-only suite passes **335/335**.
- The x64 stream smoke was repeated twenty times after one non-reproducible
  earlier parallel-suite record; all twenty passed, followed by the clean
  335/335 full rerun. No source change was made for an unproven race.
- NXVM and MyNES documentation governance both pass.

## NXVM gate recovery

The import exposed four stale post-layout static-gate inputs and one omitted
residual-ledger row. NXVM P2 corrects only those inputs: the session inventory
path now names `tools/nxvm`, product CMake verifiers read
`cmake/nxvm/NxvmProduct.cmake`, the C-facade gate inspects headers rather than
private `.c` implementation leaves, and the T345 ledger classifies the already
compiled `vm-app|src/app-nxvm/product/startup.c` residual. `banner.h` no longer
exports a C runtime header. These are gate/ABI-boundary corrections, not audio
or machine behavior changes.
