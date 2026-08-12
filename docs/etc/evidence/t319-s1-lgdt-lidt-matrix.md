# M5 T319 S1: LGDT/LIDT Matrix

## Scope

This record covers only Intel 80286/80386 `LGDT m16&24/m16&32` (`0F 01 /2`)
and `LIDT m16&24/m16&32` (`0F 01 /3`) outside VM86.  The active packet owns
this bounded load slice.  VM86 `#GP(0)` producer/delivery proof is transferred
to the Queue's VM86-to-protected exception and interrupt delivery foundation;
80286 LOCK handling remains the separate legacy-prefix debt.

## Form Matrix

| Boundary | Owner proof |
| --- | --- |
| Real/protected CPL0 success | `core-machine-lgdt-lidt-smoke` runs both forms on 80286 and 80386 in real and protected CPL0 state, with distinct limit/base images, exact instruction length, and GPR/FLAGS/cache preservation. |
| 80386 attributes | Default, `66`, `67`, and `66 67` forms prove 16/32-bit pseudo-descriptor base width and 16/32-bit effective-address routing. |
| Address selection | DS displacement, BP/SS-default, and ES override select separate pseudo-descriptor images for both load forms. |
| Rejection | 80186 escape, memory-only register ModRM, 80286 attribute forms, and 80386 LOCK forms stop with `#UD` and no CPU publication. |
| Source/table atomicity | A protected DS source limit permits the first two bytes but blocks the trailing dword.  The no-IDT terminal `#DF` boundary retains restart EIP, CPU state, and both old table registers for each form. |
| Protected CPL>0 | Owner-local GDT/IDT fixture proves `/2` and `/3` raise delivered `#GP(0)` before source pseudo-descriptor access or GDTR/IDTR publication.  The saved fault point is EIP zero; a same-CPL NOP handler advances independently. |
| PIC ordering | Pending IRQ0 after each successful real-mode load is delivered at saved IP five, reaches the HLT handler, moves IRQ0 IRR to ISR, and proves no instruction shadow. |
| Post-load consumers | `LIDT` is consumed by the real-mode IRQ0 delivery vector.  A protected CPL0 `LGDT` vector then executes `MOV DS,AX` against a descriptor available only through the newly loaded GDT and proves the loaded DS cache. |

## Production Change And Sweep

`INS_0F_01` now rejects only non-VM86 protected CPL>0 `/2` and `/3` before
ModRM decode or source read.  `_s_load_gdtr` and `_s_load_idtr` retain their
existing post-read guards for every other caller.  The sweep is:

```text
rg -n -C 12 "INS_0F_01|_s_load_gdtr|_s_load_idtr|_d_modrm_table_memory" src/core/machine/cpu_instructions.c
rg -n -C 8 "LGDT|LIDT|SGDT|SIDT|SMSW|LMSW" src/core/machine tests/machine CMakeLists.txt docs
```

`/0,/1` remain T318 stores, `/4,/6` remain T316 S63, `/5,/7` remain `#UD`,
and VM86 delivery, shared paging/memory, generic exception delivery, and
legacy LOCK policy remain outside this S.

## Verification

The owner marker is `M5:T319:S1:LGDT-LIDT:OK`; the current-gate registration
is exactly `current.core-machine-lgdt-lidt-smoke`.  Final fresh configuration,
artifact identity/hash, governance, diff check, and complete current gate are
recorded in T319 history at coordinator acceptance.  The accepted developer
artifact is `build/output/nxvm_0_5_0319.exe` (SHA-256
`5F9E0AEBFEE90E8C8F5337BA48E86BC699935AE1AAAEB517E07EBCD286476ABF`).
