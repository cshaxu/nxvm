# M5 T539 S69 P2 -- 8086 Timing Receiver Map

## Scope

S69 P2 gives the 8086 timing corpus the same Core-machine receiver naming
used by the accepted common timing baseline. It does not alter an instruction
recipe, timing formula, generated manifest row, decoder inventory or public
production API.

| Former source | Sole receiver source |
| --- | --- |
| `core_machine_8086_instruction_timing_ledger_smoke.c` | `machine_8086_instruction_timing_ledger_smoke.c` |
| `core_machine_8086_timing_manifest_runner.c` | `machine_8086_timing_manifest_runner.c` |

The manifest receiver is intentionally compiled twice: once for the 8086
catalog and once for the 8088 catalog. The two target names express distinct
frozen profiles while the single receiver source preserves one recipe executor
and one generated-catalog path.

| Former target | Receiver target |
| --- | --- |
| `core-machine-8086-instruction-timing-ledger-smoke` | `machine-8086-instruction-timing-ledger-smoke` |
| `core-machine-8086-timing-manifest-runner` | `machine-8086-timing-manifest-runner` |
| `core-machine-8088-timing-manifest-runner` | `machine-8088-timing-manifest-runner` |

## Verification

- Focused x86/x64 ledger, 8086 manifest, 8088 manifest, 8086 result, 8088
  result and 8086 decoder-ledger tests passed. The manifest result verifies
  1,053 Table-2-21 keys and the decoder ledger reports zero difference.
- Complete repository-only unit suites passed 426/426 on x86 in 86.82 seconds
  and 426/426 on x64 in 83.89 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- No Shared source, firmware, asset, INI or EXE input changed in P2; MyNES was
  neither built nor modified, and no product EXE rebuild is required.
