# M5 T539 S56c3 — TSS32 nesting receiver map

| Original context | Receiver | Observation |
| --- | --- | --- |
| Nested direct, operand-size and indirect `CALL` | `cpu_task_switch32_state_smoke` | target TR `0030h`, busy source/target descriptors, backlink `0028h`, `NT`, target EAX and HLT |
| Direct task-gate `CALL`; direct and operand-size task-gate `JMP` | `cpu_task_switch32_state_smoke` | target TR/EIP/EAX and the `CALL` nesting state where applicable |
| Nested `IRET` return | `cpu_task_switch32_state_smoke` | resumed source TR/EAX plus the outgoing target-TSS saved state |
| Nested invalid-code, busy-target, short-target and stack-limit rejection | `cpu_task_switch32_state_smoke` | exact `#TS`/`#GP` delivery, retained source state and descriptor busy state |

The receiver creates ordinary guest GDT/LDT/TSS images in copied CPU memory and
executes real `LGDT`, `LMSW`, `LTR`, task transfers and `IRET`.  It owns no
Core machine, PIC, board mapping or private CPU-cache setup.  The task-gate
descriptor is now a correctly aligned eight-byte GDT entry; the LDT short-TSS
case changes the actual GDT LDT-descriptor limit, rather than an unrelated LDT
byte.  Its local case classifiers are bounded so LDT, debug/LOCK and later
nested cases cannot leak into one another.

The retained mixed `core_machine_task_switch_smoke.c` no longer invokes these
eleven rows.  It retains only S56c4's two pending-IRQ rows and the S59 timing
manifest recipe.  There is one CPU-local execution path for every S56c3
context, with no alternate task-state fixture.

This is test/documentation-only work.  No production/API, Shared, firmware,
asset, INI or executable input changes; an EXE rebuild is not required.

Focused x64/x86 receivers and the retained task-switch corpus pass.  Complete
repository-only x64/x86 unit suites pass 426/426 serially; the required T317,
T332, T344, Core CPU/PIC-authority and lifecycle gates, documentation
governance and `git diff --check` pass on both widths.
