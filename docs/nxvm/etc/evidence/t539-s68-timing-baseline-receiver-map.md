# M5 T539 S68 -- Common Timing Baseline Receiver Map

## Scope

S68 is the bounded common timing baseline from the 17-file, 20,417-line
timing corpus intake. It changes receiver identity only: timing algorithms,
formula rows, measured deltas and production APIs are unchanged.

| Former receiver | Sole Core-machine receiver |
| --- | --- |
| `core_machine_instruction_timing_smoke.c` | `machine_instruction_timing_smoke.c` |
| `core_machine_instruction_timing_ledger_smoke.c` | `machine_instruction_timing_ledger_smoke.c` |
| `core_machine_legacy_timing_normalization_s2_smoke.c` | `machine_legacy_timing_normalization_s2_smoke.c` |
| `core_machine_t359_s2_timing_smoke.c` | `machine_t359_s2_timing_smoke.c` |
| `core_machine_t359_s3_timing_smoke.c` | `machine_t359_s3_timing_smoke.c` |
| `core_machine_t359_s4_timing_smoke.c` | `machine_t359_s4_timing_smoke.c` |
| `core_machine_t359_s5_timing_smoke.c` | `machine_t359_s5_timing_smoke.c` |
| `core_machine_t359_s6_timing_smoke.c` | `machine_t359_s6_timing_smoke.c` |

The receivers remain Core-machine tests because they observe the execution
provider, elapsed guest time, ports and protected-board behavior. No private
CPU layout is newly exposed, and no alternative fixture path is introduced.

## Verification

- Focused x86 and x64 runs passed the retained T265, T357, T362 and T359 S2-S6
  markers.
- Repository-only unit suites passed 426/426 on x86 in 158.01 seconds and
  426/426 on x64 in 152.19 seconds.
- T344 registration and historical-shape, T332 lifecycle, VM-machine
  lifecycle, Core CPU/PIC authority, and T388 lexeme/physical-boundary gates
  passed on both widths.
- Documentation governance and `git diff --check` are required at closure.

No Shared component, firmware, external asset, INI or EXE input changed; no
product binary rebuild is required. MyNES was neither built nor modified.
