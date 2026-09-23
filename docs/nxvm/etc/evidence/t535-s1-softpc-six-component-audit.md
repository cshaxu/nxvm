# T535 S1: SoftPC Six-Component Frozen Audit

## Source Decision

The accepted input is SoftPC commit
`1c5a47146dd4fd87b09423b7a7b960becb50cd67` (`1c5a4714`,
`M9 T81 S8 P2: Deliver finite PC speaker pulses`). Its dirty media paths are
excluded: `assets/media/win31_en_installed.img` and
`assets/media/win31_en_installed_original.img`. The import universe is exactly
the six project-owned MIT trees named by the T535 proposal. SoftPC is read-only
comparison material; T535 writes NXVM only after this audit.

## Complete Relative-Path Inventory

Every relative file path and SHA-256 content hash was compared:

| Root | NXVM | SoftPC | Disposition |
| --- | ---: | ---: | --- |
| `src/lib` | 91 | 97 | six added files and seven changed shared/build files |
| `src/common` | 23 | 23 | `session/control_state.c` and manifest changed |
| `src/x86` | 15 | 15 | exact |
| `test/lib` | 46 | 49 | three added audio tests plus build/manifest changes |
| `test/common` | 20 | 20 | state-matrix test and manifest changed |
| `test/x86` | 10 | 10 | exact |

The nine added paths are `src/lib/audio/{stream_interface.h,stream.h,stream.c,linux/stream.c,win32/stream.c}`,
`src/lib/types/win32/audio.h`, and `test/lib/{audio_native_smoke.c,audio_stream_smoke.c,audio_win32_platform_smoke.c}`.
No SoftPC path is deleted and no NXVM-only shared path exists.

## Semantic And Build Classification

- `lib/audio` is a neutral bounded PCM stream: 22050/44100/48000 Hz mono or
  stereo S16 frames, copied submissions, explicit active/flush/clear/wait and
  platform-owned output. It depends only on existing `types` and `base`; the
  Win32 leaf links `winmm`. It has no machine, guest, profile, ROM, media or
  product state.
- The Common update prevents a paused/stopped/reset/error state from creating
  a missing Window. A previously live Window can remain through pause; only
  running creates an absent Window. Its upstream matrix adds this regression.
- x86 source and tests have zero delta. No Debug/assembler adaptation exists.
- NXVM already adds all six roots to its CMake graph. S2 replaces their
  canonical CMake/manifests; S3 verifies target linkage in NXVM. No current
  NXVM caller references `lib_audio_stream_*`, so this import does not connect
  audio to a machine or change guest audio behavior.

## Receiver And Completion Evidence

S2 copies all six roots unchanged, preserving manifests/notices/source-only
entry points. S3 changes only NXVM-owned callers outside those roots if needed.
S4 proves exact relative-path/normalized-hash equality, standalone corpus
tests, full NXVM unit/integration gates and T535 artifacts. No blocker or
owner decision is required.
