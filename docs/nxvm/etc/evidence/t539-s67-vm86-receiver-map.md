# M5 T539 S67 — VM86 Receiver Map

## Boundary

The four S67 sources require public Core-machine construction: VM86 stack and
interrupt frames, guest table loads, Core paging translation, and PIC-backed
hardware delivery are board facts. They are therefore machine receivers, not
CPU-only fixtures.

| Original source | S67 receiver | Responsibility |
| --- | --- | --- |
| `core_machine_vm86_delivery_smoke.c` | `machine_vm86_delivery_smoke.c` | VM86 frame delivery and fault rollback |
| `core_machine_vm86_iret_smoke.c` | `machine_vm86_iret_smoke.c` | VM86 IRET frame validation and return |
| `core_machine_vm86_lgdt_lidt_s5_smoke.c` | `machine_vm86_lgdt_lidt_s5_smoke.c` | VM86 table-load and paging-fault cases |
| `core_machine_hardware_delivery_s3_smoke.c` | `machine_hardware_delivery_s3_smoke.c` | direct includer of VM86 delivery with hardware IRQ coverage |

No production CPU, public API, Shared component, firmware, asset, INI or EXE
input changes. The renamed tests retain their prior fixture bodies; the delivery
receiver additionally emits `M5:T539:S67:VM86:OK` as this package marker.

## Verification

- Focused x86/x64 runs: the four receivers pass. The delivery receiver emits
  `M5:T539:S67:VM86:OK`; retained markers are `M5:T320:S1:VM86-DELIVERY:OK`,
  `M5:T320:S2:VM86-IRET:OK`, `M5:T321:S5:VM86-LGDT-LIDT:OK`, and
  `M5:T321:S3:HARDWARE-DELIVERY:OK`.
- Repository-only unit suites: x86 426/426 passed (177.93 s); x64 426/426
  passed (175.28 s).
- On x86 and x64: T344 registration, T344 fixture shapes (101 inventoried
  direct fixtures), T332 lifecycle, VM-machine lifecycle and Core CPU/PIC
  authority pass.
- Documentation governance and `git diff --check` pass. MyNES is not built or
  modified by this package.
