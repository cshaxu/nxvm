# M5 T539 S59 — TSS32 paging receiver map

| Original context | Receiver | Owner | Observation |
| --- | --- | --- | --- |
| TSS32 task jump loads a paging target | `machine_task_switch32_paging_smoke` | public Core | target TR `0030h`, CR3 `00004000h`, EAX `00001234h`, then HLT |
| Target TSS itself crosses an unmapped page | `machine_task_switch32_paging_smoke` | public Core | delivered `#PF`, retained source TR `0028h`/CR3 `00001000h`, then the real IDT handler HLT |

These are the two S59 rows because paging translation, page-table memory and
page-fault delivery belong to the Core machine's public physical-memory and
execution contracts.  The receiver uses only the public machine lifecycle,
entry plan, physical-memory writes, `core_machine_run()`, diagnostic record and
copied CPU snapshot.  It creates the guest GDT/IDT/TSS/page tables in ordinary
RAM and executes real `LGDT`, `LIDT`, `LMSW`, `LTR`, and task-jump instructions.
It does not reach an executor CPU, fabricate a TR/IDTR cache, or emulate a
translation outcome in the fixture.

One Core run budget may return `CORE_MACHINE_STOP_BUDGET` immediately after it
commits an exception delivery.  The receiver therefore performs one further
bounded public run to execute the already-delivered `#PF` handler through HLT.
That is Core's documented public execution boundary, not a second exception
path or a test-only Core API.

`core_machine_task_switch_smoke.c` no longer contains S59 paging state,
private IDTR mutation, direct page-table setup, or paging assertions.  Its
remaining TSS task-switch corpus is retained for rows owned by later packages
and the S65 timing manifest include. The original exceptional group is otherwise
partitioned without overlap: S58 owns debug-trap/LOCK, S59 owns paging,
S60 owns nesting/task-gate/return, and S61 owns pending PIC delivery.

This is test/CMake/documentation-only work.  No production/API, Shared,
firmware, asset, INI or executable input changes; an EXE rebuild is not
required.

Focused receiver and retained task-switch corpus pass on x64 and x86.  The
complete repository-only unit suites pass 426/426 on each width when run
serially.  Serial execution is intentional: the pre-existing CPU bus boundary
negative gate uses a fixed probe path and is not parallel-safe; it passes in
the complete serial suite on both widths.  T317, T332, T344, Core CPU/PIC
authority and lifecycle-ownership gates pass on both widths; documentation
governance and `git diff --check` pass.  The code change adds one 168-line
public receiver and removes the residual paging fixture paths; there is no
production-code delta.
