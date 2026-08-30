# T504 S5 Integration Closure

## Contained inputs

All replays used owner-managed BYOB firmware or DOS media read directly from
the external asset archive. No asset path, hash, bytes, raw trace, or guest
data is retained in this repository. Release executables were rebuilt locally;
the runtime debugger was retained and no trace mode was enabled.

## Same-input results

| Workload | Standard terminal / wall time | Turbo terminal / wall time |
| --- | --- | --- |
| Default PC/AT with registered external DOS boot disk | `A:\\>` / 3185.9 ms | `A:\\>` / 2815.8 ms |
| DeskPro Model 40 with owner-supplied Compaq ROM pair, 1.2-MB DOS disk, and documented F1 continuation | `A:\\>` / 52629.5 ms | `A:\\>` / 52878.4 ms |

Turbo is materially faster on the paced default-PC/AT replay (about 11.6% in
this run). The unqualified Model-40 replay is semantically identical but not
faster in this sample; its Standard mode has no physical/macro pacing wait to
remove. These are host measurements, not a guest-time claim or benchmark
guarantee.

## Boundary conclusion

Both modes retain Core-owned deadline progression. Supported profile replays
observed sourced PIT deadlines rather than a manufactured L1 disposition; the
Core-only source-less fixture remains the proof for the bounded L1 transition.
No host time was converted to guest ticks, no controller-specific VM route was
introduced, and no external input crossed into repository source or evidence.

The prior short Debug probe did not reach the Model-40 terminal, while the
required stripped Release replay did. Release is the product verification
surface; Debug remains a development/unit-gate configuration.

## Artifact

The local stripped Release artifact is `nxvm_0_5_0504.exe`, SHA-256
`6B4C9D047473488807242674DE479151B74BE1DD72625F0F9AC31F3378527443`.
