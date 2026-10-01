# M5 T539 S72 -- 80286 Manifest And Call-Gate Receiver Map

## Scope

S72 renames the 80286 timing-manifest runner and its direct call-gate
includer. It does not change an instruction recipe, timing formula, generated
manifest row, call-gate construction, decoder inventory or public production
API.

| Former source | Sole Core-machine receiver |
| --- | --- |
| `core_machine_80286_timing_manifest_runner.c` | `machine_80286_timing_manifest_runner.c` |
| `core_machine_call_gate_smoke.c` | `machine_call_gate_smoke.c` |

| Former target | Receiver target |
| --- | --- |
| `core-machine-80286-timing-manifest-runner` | `machine-80286-timing-manifest-runner` |
| `core-machine-call-gate-smoke` | `machine-call-gate-smoke` |

## Verification

- Focused manifest and call-gate receiver tests passed on x86 and x64.
- Complete repository-only unit suites passed 426/426 on x86 in 105.59
  seconds and 426/426 on x64 in 105.52 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- No Shared source, firmware, asset, INI or EXE input changed; MyNES was
  neither built nor modified, so no product EXE rebuild is required.
