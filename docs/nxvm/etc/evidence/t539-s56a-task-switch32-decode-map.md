# M5 T539 S56a — 80386 task-JMP decode receiver map

| Original context | Receiver | Owner | Observation |
| --- | --- | --- | --- |
| `66 EA ptr16:32, TSS16` | `cpu_task_switch32_decode_smoke` | CPU | saved IP `000B`, target TSS/AX and halt |
| `66 FF /5 m16:32` | `cpu_task_switch32_decode_smoke` | CPU | saved IP `0008`, target TSS/AX and halt |
| `67 FF /5 m16:16` | `cpu_task_switch32_decode_smoke` | CPU | saved IP `000A`, target TSS/AX and halt |
| `66 67 FF /5 m16:32` | `cpu_task_switch32_decode_smoke` | CPU | saved IP `000B`, target TSS/AX and halt |

The receiver configures CPU-owned memory, descriptors and TSS images using the
existing S55 fixture. It links only `x86-cpu`; it creates no board, PIC, VM or
profile state.

`core_machine_task_switch_smoke.c` no longer invokes these four functional
contexts. Its encoding selector and byte recipes remain because the S59 80386
timing manifest runner includes that source to derive timing-only recipes.
That downstream inclusion is not a second functional receiver.
