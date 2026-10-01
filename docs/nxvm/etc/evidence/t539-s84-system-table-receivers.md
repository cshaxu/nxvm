# T539 S84 System-Table Receiver Map

## Scope

S84 moves the four CPU-only descriptor-table instruction receivers from NXVM
to `test/x86/devices/cpu/`:

| Former NXVM source | Shared receiver | Lines |
| --- | --- | ---: |
| `cpu_descriptor_system_smoke.c` | `cpu_descriptor_system_smoke.c` | 545 |
| `cpu_dttr_s61_smoke.c` | `cpu_dttr_s61_smoke.c` | 168 |
| `cpu_lgdt_lidt_smoke.c` | `cpu_lgdt_lidt_smoke.c` | 149 |
| `cpu_sgdt_sidt_smoke.c` | `cpu_sgdt_sidt_smoke.c` | 206 |

The batch is 1,068 source lines. Public board paths, including
`core_machine_descriptor_system_smoke.c` and
`machine_table_register_board_smoke.c`, remain NXVM owners.

## Boundary

`cpu_descriptor_system_smoke.c` formerly imported
`app-nxvm/devices/device_support.h` only for CR0 protected-mode bit testing
and setting. Its Shared successor uses direct equivalent local expressions;
no App include, public API, fixture, or production path crosses the boundary.

## Verification

- Focused Shared successors pass on x64 and x86.
- Complete 437-case x64 and x86 unit runs completed. The x64 parallel run
  recorded five existing string-test flakes, and x86 recorded one `cpu_movs`
  flake; all six pass on their first isolated rerun. The x64 serial run
  recorded `unit.vm-runner-error-propagation-smoke`, which likewise passes in
  isolation. These unrelated runners are recorded rather than attributed to
  this test-only migration.
- Shared manifest/corpus, Core CPU/PIC authority, documentation governance and
  `git diff --check` pass.

No executable input changed; artifacts are intentionally unchanged.
