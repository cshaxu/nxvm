# M5 T539 S64 — CLI/STI receiver map

| Original context | Sole receiver | Preserved observation |
| --- | --- | --- |
| Real-mode CLI/STI over 8088, 8086, 80186 and 80386 | `machine_cli_sti_interrupt_smoke.c` | IF clearing/setting and non-IF flag preservation |
| STI one-instruction shadow, PIC mask and IRQ acknowledgment | same | real PIC IRR-to-ISR state and actual interrupt frame |
| 8088 PIC, keyboard comparison, PIT IRQ and RAM POST sequences | same | guest port/memory operations and final guest-visible markers |
| Protected and VM86 CLI/STI privilege paths | same | real Core fault and delivered-exception observations |
| 80286 extension cases | `core_machine_cli_sti_s48_smoke.c` | inherited CLI/STI lifecycle with its own S48 behavior |
| HLT wake, prefix and IRQ cases | `core_machine_hlt_s49_smoke.c` | inherited CLI/STI lifecycle with its own HLT behavior |
| INT-to-IRET-to-IRQ composition | `core_machine_interrupt_return_composition_s4_smoke.c` | inherited lifecycle and public PIC/frame observation |

`machine_cli_sti_interrupt_smoke.c` is the sole named Core-machine receiver
for the common CLI/STI construction. The three direct S64 includers reuse it;
they do not create another execution provider, machine lifecycle or IRQ setup.
The S65 IRET and S66 software-INT sources retain a mechanical include update
only, so their future ownership remains unchanged.

Focused x64 and x86 runs pass for the receiver and all three direct includers,
and the receiver emits `M5:T539:S64:CLI-STI-INTERRUPT:OK`. Final complete
repository-only units pass 426/426 on x64 (334.05 seconds) and x86 (161.67
seconds). T332 lifecycle, T344 registration and fixture-shape, VM lifecycle,
CPU/PIC authority, documentation governance and `git diff --check` pass on
both widths. This is test/CMake/documentation-only work; no production/API,
Shared, firmware, asset, INI or executable input changes, so no EXE rebuild is
required.
