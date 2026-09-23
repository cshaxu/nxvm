# M2 T7 Debugger Closure Evidence

The accepted debugger workflow is complete through one Core machine and
Common's existing paused-lease rendezvous. The protocol supplies copied
OBSERVE, PEEK, POKE, STEP, BREAK_SET, BREAK_LIST and WARM_RESET records;
OUTPUT_SET and BIND_SET validate their fixed layouts then return unsupported.
The Console commands use that protocol for register, memory, poke, disassembly,
step, breakpoint and soft-reset workflows.

`debug_workflow_smoke` proves valid records plus version, length, capacity,
unsupported-operation, breakpoint-capacity and stale-lease rejection paths.
It also proves breakpoint hit, single bypass and rearm using the Common worker
and production CPU. `app_debug_commands_smoke` proves actual parser/output and
state-gating behavior. The App opcode rendering table contains exactly the 151
entries in `cpu-opcodes.csv`; the closure sweep compares both sets with no
missing or extra opcode.

The complete 89-test CTest suite passed from both shared x64 and x86 trees.
Current product artifacts report `pei-x86-64` and `pei-i386`; their SHA-256
values are respectively
`A326A7CD3F08A648D2015F04F224E4E0885E916C88C80273A0F032F93AAA9C40` and
`68360DE2E930FDF98957CFDC0FB8D7882D33248DA7D6CEBEEF4FE8C8456E4BC9`.

Ordered bus-transfer timing and cycle-sampled reset/interrupt behavior transfer
to the next timing package. Cross-operation fault injection, strict broader
monitor grammar and lifecycle reliability transfer to the final reliability
package; they do not weaken the implemented debugger contracts.
