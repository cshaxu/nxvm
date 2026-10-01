# M5 T539 S70 -- 80186 Timing Receiver Map

## Scope

S70 renames the two 80186 timing receivers; it does not change an instruction
recipe, timing formula, generated manifest row, decoder inventory or public
production API.

| Former source | Sole Core-machine receiver |
| --- | --- |
| `core_machine_80186_instruction_timing_ledger_smoke.c` | `machine_80186_instruction_timing_ledger_smoke.c` |
| `core_machine_80186_timing_manifest_runner.c` | `machine_80186_timing_manifest_runner.c` |

| Former target | Receiver target |
| --- | --- |
| `core-machine-80186-instruction-timing-ledger-smoke` | `machine-80186-instruction-timing-ledger-smoke` |
| `core-machine-80186-timing-manifest-runner` | `machine-80186-timing-manifest-runner` |

## Verification

- Focused ledger and manifest receiver tests passed on x86 and x64.
- Complete repository-only unit suites passed 426/426 on x86 in 102.58
  seconds and 426/426 on x64 in 102.56 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- No Shared source, firmware, asset, INI or EXE input changed; MyNES was
  neither built nor modified, so no product EXE rebuild is required.
