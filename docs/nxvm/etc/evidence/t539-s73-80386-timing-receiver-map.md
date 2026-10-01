# M5 T539 S73 -- 80386 Timing Receiver Map

## Scope

S73 renames the 80386 timing-manifest runner. It does not change an
instruction recipe, timing formula, generated manifest row, decoder inventory
or public production API.

| Former source | Sole Core-machine receiver |
| --- | --- |
| `core_machine_80386_timing_manifest_runner.c` | `machine_80386_timing_manifest_runner.c` |

| Former target | Receiver target |
| --- | --- |
| `core-machine-80386-timing-manifest-runner` | `machine-80386-timing-manifest-runner` |

## Verification

- Focused timing-manifest receiver tests passed on x86 and x64.
- Complete repository-only unit suites passed 426/426 on x86 in 98.16
  seconds and 426/426 on x64 in 98.08 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- No Shared source, firmware, asset, INI or EXE input changed; MyNES was
  neither built nor modified, so no product EXE rebuild is required.
