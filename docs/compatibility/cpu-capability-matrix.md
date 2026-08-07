# CPU Capability Evidence Matrix

This matrix records project-owned evidence, not an x86 marketing label. A
capability remains unavailable to a profile unless its required instruction,
fault, prefix/addressing, register/FLAGS, and side-effect probes have passed.

| Capability | Current verdict | M5 T1 S1 evidence | Remaining gate |
| --- | --- | --- | --- |
| `x86.real_mode_8086` | partial evidence; not yet profile-claimable | one-step immediate MOV, immediate ADD, relative jump, segment-prefix NOP, and `#UD` capture on the retained baseline | broaden instruction, memory, stack, interrupt, FLAGS, and side-effect matrix; cross-check applicable cases |
| `x86.protected_mode_286` | bounded partial evidence; not profile-claimable beyond the admitted subset | T257/T259: GDT-only 16-bit `LGDT`/`LMSW`, selector loads, same-CPL far transfer, CPL3 software-gate entry, 16-bit TSS `SS0:SP0` switch, outer `IRET`, and delivered `#GP` observation | CPL3 paging permissions, generic IDT/hardware delivery, LDT, task switching, 32-bit semantics, and broader descriptor/fault corpus; 80286 has no I/O map |
| `x86.i386_decode` | partial evidence; not yet profile-claimable | operand-size-prefixed immediate MOV | broader `0F` and prefix/decode matrix with positive and negative cases |
| `x86.i386_real_mode` | unknown | none | evidence-backed 32-bit real-mode execution matrix |
| `x86.i386_protected_mode` | bounded partial evidence; not profile-claimable beyond the admitted 16-bit subset | T259 runs the 16-bit CPL3 gate/outer-`IRET`/delivered-`#GP` corpus; T260 adds 32-bit-TSS I/O-map allow/deny and bounds checks under the 80386 profile | 32-bit frames/gates, CPL3 paging, generic IDT/hardware delivery, task switching, and broader corpus |
| `x86.i386_paging` | bounded CPL0 partial evidence; not profile-claimable beyond the admitted path | T258: 4 KiB PDE/PTE walk through core physical memory; fetch/data/stack mapping; A/D updates; non-present `#PF` with CR2; narrowed CR0/CR2/CR3 forms | CPL3 P/W/U faults, broader protected IDT delivery, TLB behavior, PSE/PAE, TSS, task switching, and broader 32-bit corpus |
| `fpu.esc` | unavailable; contract design active | `MEM` reaches `FNINIT` (`DB E3`) but current escape dispatch raises `#UD` | T153--T158 define per-machine FPU profiles, then verify ESC consumption and CR0 `EM`/`TS`/`MP` behavior |

The baseline probe harness records an instruction window of at most 15 bytes,
CS:IP and linear PC, EAX/EBX/ECX/EDX, FLAGS, and exception mask/code. It has no
guest-media dependency. The local MS-DOS `MEM` observation is still pending a
separate owner-local bounded reproducer; it is not represented by this matrix.
