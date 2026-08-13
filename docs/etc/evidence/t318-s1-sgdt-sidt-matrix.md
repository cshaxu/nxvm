# M5 T318 S1: SGDT/SIDT Matrix

## Scope And Authority

This record covers only Intel 80286/80386 `SGDT` (`0F 01 /0`) and `SIDT`
(`0F 01 /1`) memory pseudo-descriptor stores.  The Intel 80386 Programmer's
Reference Manual system-instruction entries are the behavioral authority.  The
T316 S66 handoff transferred this table-register family to the later
processor-control package; its withdrawn table-register smoke is not evidence.

The retained executor is `INS_0F_01` in
`src/core/machine/cpu_instructions.c`.  `_d_modrm_table_memory` continues to
reject register ModRM before effective-address use.  This S1 changes only the
two store callers: `_m_write_table_pseudo_descriptor` first preflights the
complete six-byte destination and then publishes the one image through the
existing logical-memory route.  The original two-write sequence could publish
the limit before the later four-byte access faulted; the protected write-limit
probe reproduced that risk.  No shared decoder, memory, exception, paging, or
interrupt route changed.

## Form Matrix

| Form/boundary | Focused proof and disposition |
| --- | --- |
| `SGDT` and `SIDT`, 80286 real and protected mode | `core-machine-sgdt-sidt-smoke` executes both default-size memory forms in each profile/mode pairing, checks distinct GDTR/IDTR limit/base images, exact EIP, and unchanged GPRs, FLAGS, and visible/cache segment state. |
| 80386 16/32-bit operand and address attributes | The four `none`, `66`, `67`, and `66 67` forms prove EIPs 5, 6, 8, and 9 respectively.  `16`-bit operand form writes `m16:24` with byte five zero; `66` writes the complete `m16:32` image. |
| Segment-selected effective address | For **each** `/0` and `/1`, DS displacement, SS-default BP displacement, and ES override prove the selected physical destination, exact EIP, six-byte image, and unchanged GPR/FLAGS/segment cache state. |
| Memory-only/rejection | For **each** `/0` and `/1`, 80386 register ModRM and representative default-form F0 LOCK reach native `#UD` without CPU state or destination publication. Reserved `/5` is retained `#UD`; each `/0` and `/1` memory form reaches `#UD` without publication on 80186. The 8086 legacy `0F` POP-CS compatibility route is retained outside this 80286-or-later decoder and is not SGDT/SIDT admission. |
| Protected destination boundary | For **each** `/0` and `/1`, a protected DS limit ending inside the six-byte destination faults before any byte changes; all six sentinel bytes remain intact, EIP restarts at zero, and captured GPR/FLAGS/segment cache state remains unchanged. |
| Pending PIC ordering | For **each** `/0` and `/1`, a pending master IRQ before a successful store is delivered after the actual instruction (saved frame IP is 5), reaches the HLT handler, clears IF, moves the request into PIC ISR, clears IRR, and leaves the table image correct. Neither form receives an interrupt shadow. |
| VM86 | For **each** `/0` and `/1`, 80386 VM86-state execution stores the correct image at DS, advances EIP to 5, and preserves captured GPR/FLAGS/segment cache state. This is a factual implementation/PRM classification, not V86 breadth admission. |

## Similar-Issue Sweep

The S1 sweep used:

```text
rg -n -C 12 "INS_0F_01|_d_modrm_table_memory|_m_write_rm" src/core/machine/cpu_instructions.c
rg -n -C 8 "SGDT|SIDT|LGDT|LIDT|SMSW|LMSW" src/core/machine tests/machine CMakeLists.txt docs
```

`/0` and `/1` are fixed by the local six-byte helper.  `/2` LGDT and `/3`
LIDT retain their existing two-read candidate path and are deferred by the
active packet. `/4` SMSW and `/6` LMSW retain their fixed-word behavior and
T316 S63 evidence. `/5` and `/7` remain `#UD`.  No other caller of
`_d_modrm_table_memory` changes in this S1; altering their source/destination
commit rules would exceed the approved table-store scope.

## Verification And Artifact

- Fresh GCC configuration and the standalone focused target passed.
- The owner smoke emits `M5:T318:S1:SGDT-SIDT:OK`.
- `PROJECT_CURRENT_SMOKE_TARGETS` contains exactly one
  `core-machine-sgdt-sidt-smoke` registration; the normal current-gate helper
  derives its single CTest registration from that list.
- P2 reruns fresh configuration, the exact registration discovery, governance,
  diff, full current gates, and the current artifact SHA-256/runtime identity;
  the corresponding T318 history records the result.

## Boundary

This record does not admit LGDT/LIDT, selector/task forms, MSW/control
registers, paging, generic delivery work, V86 breadth, legacy LOCK policy, or
any x87 capability.
