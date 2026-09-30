# T539 S44 Descriptor-query migration map

This record keeps instruction semantics separate from board delivery.  The two
legacy receivers are not removed until every row has a single receiving test.

| Legacy group | Receiving owner | Status |
| --- | --- | --- |
| LAR/LSL register, selector, presence, DPL/RPL/CPL and ZF | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL 16/32-bit operand and direct memory forms | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL BP-default-SS and ES/FS/GS forms | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL 67/SIB source | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL source-limit and rollback | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL VM86 and LDT selector | `cpu-lar-lsl-smoke` | Migrated |
| LAR/LSL timing | `core-machine-80386-timing-manifest-runner` | Migrated: real protected guest-table rows cover 21/25/22/26 source ticks |
| VERR/VERW selector outcomes, prefix, memory and ZF preservation | `cpu-verr-verw-smoke` | Migrated |
| VERR/VERW source-limit and rollback | `cpu-verr-verw-smoke` | Migrated |
| VERR/VERW LDT selector | `cpu-verr-verw-smoke` | Migrated |
| VERR/VERW VM86 | `cpu-verr-verw-smoke` | Migrated |
| PIC delivery/no-shadow | existing public Core IRQ receivers (for example `core-machine-gpr-mov-smoke`) | Existing board receiver |
| Real guest descriptor-table setup | `core-machine-descriptor-system-smoke` | Existing board receiver |

The CPU receivers link only `x86-cpu`.  They use a local bus and, where a
loaded protected-mode cache is required, execute the same CPU instructions
that establish it; no Core-machine, PIC, firmware or profile state is used.

The timing runner is the single board owner for the page-granular descriptor:
it installs the descriptor through the guest GDT and observes a single
retirement for register and memory LSL. It does not create a second mixed
fixture or duplicate the CPU receiver's value assertions.

Verification: x86 and x64 repository-only unit suites pass 416/416; both
specialized aggregates pass, and the x86 focused timing runner passes.
