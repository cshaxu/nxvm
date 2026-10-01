# M5 T539 S71 -- 80286 Ledger And Protected-I/O Timing Receiver Map

## Scope

S71 renames the 80286 timing ledger and 80386 protected-I/O timing receivers.
It does not change an instruction recipe, timing formula, protected-port row,
generated manifest row, decoder inventory or public production API.

| Former source | Sole Core-machine receiver |
| --- | --- |
| `core_machine_80286_instruction_timing_ledger_smoke.c` | `machine_80286_instruction_timing_ledger_smoke.c` |
| `core_machine_80386_protected_io_timing_smoke.c` | `machine_80386_protected_io_timing_smoke.c` |

| Former target | Receiver target |
| --- | --- |
| `core-machine-80286-instruction-timing-ledger-smoke` | `machine-80286-instruction-timing-ledger-smoke` |
| `core-machine-80386-protected-io-timing-smoke` | `machine-80386-protected-io-timing-smoke` |

## Verification

- Focused ledger and protected-I/O receiver tests passed on x86 and x64.
- Complete repository-only unit suites passed 426/426 on x86 in 103.32
  seconds and 426/426 on x64 in 103.50 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- No Shared source, firmware, asset, INI or EXE input changed; MyNES was
  neither built nor modified, so no product EXE rebuild is required.
