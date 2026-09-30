# M5 T539 S42 table-register migration map

This working map records receiver ownership before the three original private
machine smokes are retired.  It is not S42 acceptance evidence.

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
| LGDT/LIDT post-load descriptor consumer | board receiver | pending |
| LGDT/LIDT protected CPL rejection and exception gate | board receiver | pending |
| SGDT/SIDT 286/386 store images, 66h/67h attributes | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT real segment routes and VM86 store context | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT invalid forms | `cpu-sgdt-sidt-smoke` | migrated |
| SGDT/SIDT protected-limit atomicity | `machine-table-register-board-smoke` | migrated |
| SGDT/SIDT DOS 80286/80386 discriminator | `machine-table-register-board-smoke` | migrated |
| All four instructions with a pending PIC IRQ | `machine-table-register-board-smoke` | migrated |

The original three sources remain the baseline until every pending board row
has a public-machine receiver.  No row may be dropped during retirement.

Current x64 interim check: the three baseline smokes and the four new
receivers pass together (7/7); the complete repository-only x64 unit set was
then run with 418 registered unit tests.  These results are interim only:
S42 remains open until the pending board rows replace the private sources.
