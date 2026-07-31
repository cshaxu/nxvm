# CPU Capability Evidence Matrix

This matrix records project-owned evidence, not an x86 marketing label. A
capability remains unavailable to a profile unless its required instruction,
fault, prefix/addressing, register/FLAGS, and side-effect probes have passed.

| Capability | Current verdict | M5 T1 S1 evidence | Remaining gate |
| --- | --- | --- | --- |
| `x86.real_mode_8086` | partial evidence; not yet profile-claimable | one-step immediate MOV, immediate ADD, relative jump, segment-prefix NOP, and `#UD` capture on the retained baseline | broaden instruction, memory, stack, interrupt, FLAGS, and side-effect matrix; cross-check applicable cases |
| `x86.protected_mode_286` | unknown | none | dedicated mode, descriptor, fault, and interrupt matrix |
| `x86.i386_decode` | partial evidence; not yet profile-claimable | operand-size-prefixed immediate MOV | broader `0F` and prefix/decode matrix with positive and negative cases |
| `x86.i386_real_mode` | unknown | none | evidence-backed 32-bit real-mode execution matrix |
| `x86.i386_protected_mode` | unknown | none | evidence-backed protected-mode execution matrix |
| `x86.i386_paging` | unknown | none | paging translation, permission, and fault matrix |

The baseline probe harness records an instruction window of at most 15 bytes,
CS:IP and linear PC, EAX/EBX/ECX/EDX, FLAGS, and exception mask/code. It has no
guest-media dependency. The local MS-DOS `MEM` observation is still pending a
separate owner-local bounded reproducer; it is not represented by this matrix.
