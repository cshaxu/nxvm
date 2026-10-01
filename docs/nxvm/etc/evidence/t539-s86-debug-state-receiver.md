# T539 S86 Debug-State Receiver

## Receiver map

The 257-line `cpu_debug_state_smoke.c` covers CPU-local MOV-DR, debug
exception and data-breakpoint behavior. It now has one owner:

`test/x86/devices/cpu/cpu_debug_state_smoke.c`

The prior NXVM source and target are deleted. The distinct
`machine_debug_state_board_smoke` remains NXVM because it observes public
machine construction and delivery, which the CPU receiver does not duplicate.

## Verification

- Focused Shared and NXVM aggregate registrations pass on x64 and x86.
- T332 resolves the canonical Shared receiver path and passes on both widths.
- The current x64 and x86 full repository-only unit logs both contain 439
  passing tests and zero failed-test records.
- CPU/PIC authority, Shared manifest/corpus, documentation governance and
  `git diff --check` pass.

No production, asset, INI or executable input changed; no artifact rebuild was
needed.
