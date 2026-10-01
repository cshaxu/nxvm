# M5 T539 S61 — TSS32 pending-IRQ receiver map

| Original context | Receiver | Observation |
| --- | --- | --- |
| Direct TSS32 task jump with a pending IRQ | `machine_task_switch32_paging_smoke` | real IRQ1 reaches the initialized 8259A after target TSS enables IF; target TR is `0030h` and the IDT handler halts at `0181h` |
| Nested TSS32 task call with a pending IRQ | `machine_task_switch32_paging_smoke` | same public IRQ1 delivery after nested target activation, with the target task remaining current |

The board receiver uses only public Core construction, reset, entry-plan,
physical-memory, execution, diagnostic, snapshot and native-keyboard APIs.
Its guest bootstrap initializes the master 8259A through ordinary I/O ports,
unmasks IRQ1, installs an IDT gate at vector `21h`, and queues one native
keyboard byte before execution.  Thus the tested route is KBC → PIC → CPU,
not a test-only PIC assertion or a borrowed Core field.

`core_machine_task_switch_smoke.c` no longer invokes either pending-IRQ row or
contains the retired direct TSS32 runner.  The S65 timing recipe remains in its
own legacy construction paths; no private TSS32/PIC execution path survives.

Focused x64/x86 receiver runs and the retained mixed corpus pass.  The full
serial repository-only unit suites pass 426/426 on both x64 and x86.  T344
historical fixture-shape and Core lifecycle ownership scripts, documentation
governance and `git diff --check` pass.  A combined CMake/Ninja batch for the
remaining static gates stalled without CPU progress on this host and is not
claimed as passing evidence.

This is test/documentation-only work. No production/API, Shared, firmware,
asset, INI or executable input changes; an EXE rebuild is not required.
