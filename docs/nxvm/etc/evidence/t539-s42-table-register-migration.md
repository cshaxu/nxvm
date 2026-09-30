# M5 T539 S42 table-register migration map

This working map records receiver ownership for the three retired private
machine smokes. It is not S42 acceptance evidence.

| Original family | Receiver | Status |
| --- | --- | --- |
| DTTR LLDT/SLDT/LTR/STR register forms, 286/386 | `cpu-dttr-s61-smoke` | migrated |
| DTTR 66h/67h attributes, null LDTR, SLDT memory store | `cpu-dttr-s61-smoke` | migrated |
| DTTR invalid forms and prefixes | `cpu-dttr-s61-smoke` | migrated |
| DTTR LTR memory form and guest descriptor execution | `machine-table-register-board-smoke` | migrated |
| LGDT/LIDT 286/386 values, 66h/67h attributes | `cpu-lgdt-lidt-smoke` | migrated |
| LGDT/LIDT invalid forms and prefixes | `cpu-lgdt-lidt-smoke` | migrated |
| LGDT/LIDT segment source | `machine-table-register-board-smoke` | migrated |
| LGDT/LIDT source-limit rollback | `machine-table-register-board-smoke` | migrated |
| LGDT post-load descriptor consumer | `machine-table-register-board-smoke` | migrated |
| LGDT/LIDT protected CPL rejection and exception gate | `machine-table-register-board-smoke` | migrated |
| SGDT/SIDT 286/386 store images, 66h/67h attributes | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT real segment routes and VM86 store context | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT invalid forms | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT protected-limit atomicity | `machine-table-register-board-smoke` | migrated |
| SGDT/SIDT DOS 80286/80386 discriminator | `machine-table-register-board-smoke` | migrated |
| All four instructions with a pending PIC IRQ | `machine-table-register-board-smoke` | migrated |

All original rows have one receiver, so the three original sources and their
registrations are retired by this change. No row was dropped.

## Implementation Verification

- Four receivers pass directly on x64 and x86:
  `M5:T539:S42:DTTR-CPU:OK`, `LGDT-LIDT-CPU:OK`,
  `SGDT-SIDT-CPU:OK`, and `TABLE-REGISTER-BOARD:OK`.
- Repository-only unit suites pass once per width: **415/415** on x64 and
  **415/415** on x86.
- The complete specialized-gate aggregate passes once per width: **66/66**.
  It includes the updated T317 35-owner strict-command audit, T332 fixture
  lifecycle audit, T344 constructor/direct-compilation audits and CPU/PIC
  authority closure.
- The product EXE relinked while x86 tests were built, but because no
  production input changed it was restored to its pre-S42 Git byte stream.

Actual-commit review and acceptance remain separate from this implementation
evidence.
