# M5 T539 S56c1 — TSS32 #DB and LOCK receiver map

`cpu_task_switch32_state_smoke.c` now owns the three instruction-local TSS32
exception families that require no board service:

- TSS debug-trap word: task-switch `#DB` delivery, saved frame, DR6 and local
  DR7-enable clearing.
- `LOCK`-prefixed direct task JMP: terminal `#UD` before task-state mutation.
- `LOCK`-prefixed indirect task JMP: the same terminal `#UD` invariant through
  the indirect decode form.

The receiver executes its own LGDT/LTR bootstrap and CPU bus. It does not
construct a Core machine, PIC, or board mapping. The residual mixed source no
longer invokes these five contexts.

The initial paging probe intentionally did **not** become a fake CPU receiver:
the CPU fixture has no Core physical-translation binding, and its CR0 write
does not activate the paging execution path (`CR0` remains `00000009`). Paging
and page-fault task-switch rows are therefore assigned to S56c2's public Core
receiver, which owns that mapping contract. This is a receiver correction, not
a functional downgrade or a new production path.

No production, API, Shared, firmware, asset, INI or executable input changes.
The changed files are test and task evidence only; no EXE rebuild is required.
