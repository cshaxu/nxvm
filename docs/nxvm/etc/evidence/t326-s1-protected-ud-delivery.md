# T326 S1: Protected Invalid-Opcode Vector-6 Delivery

## Scope

T326 S1 closes the protected-mode part of T325 S3's transferred CPU-executor
invalid-opcode delivery boundary. It does not claim real-mode IVT migration,
debug, breakpoint, task, VME/PVI, or generic interrupt-policy completion.

Intel 80386 PRM sections 9.8.6 and 9.9 define invalid opcode as restartable
fault vector 6 with no error code. The direct PRM text is retained at
<https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s09_08.htm>
and <https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s09_09.htm>.

## Reproducer And Repair

Before T326, `ExecFinal` set `exception_deliverable` for
`VCPUINS_EXCEPT_UD` only when the saved CPU had `EFLAGS.VM`. A protected
`0F 01 /7` therefore bypassed its valid IDT vector-6 gate and became a
terminal fault.

`ExecFinal` now maps `VCPUINS_EXCEPT_UD` to vector 6 independently of VM86
when protected delivery is active. The same reproducer exposed a second shared
defect: `_e_except_n` passed an error-frame request for every ordinary
protected exception. `_e_exception_has_error_code` now mechanically names only
Intel vectors 8, 10--14, and 17. This makes vector 6's protected frame exactly
`EIP`, `CS`, and `EFLAGS`; no error-code slot is written. It leaves the existing
error-code vectors explicit rather than introducing a new delivery path.

## Focused Evidence

`core-machine-protected-ud-delivery-s1-smoke` exercises all of the following
with a valid protected IDT vector-6 handler and a subsequent HLT:

| Producer class | Reproducer | Required observable |
| --- | --- | --- |
| Invalid primary opcode | `F1` | Restart IP 0, no producer publication, vector 6 delivery. |
| Reserved two-byte form | `0F 01 F8` (`/7`) | Same frame and transfer; retains T325's `INVLPG`-form observation. |
| Invalid operand form | `BOUND AX, register` (`62 C0`) | Operand-form `#UD` has the same vector-6 fault route. |
| Illegal LOCK form | `F0 90` | Prefix-originated `#UD` reaches the same route before publication. |

Protected mode proves the selected 32-bit stack frame has exactly three dwords,
restart EIP 0, pre-fault CS/EFLAGS, interrupt-gate IF clearing, handler entry,
and handler progress. Its invalid vector-6 gate case remains terminal with the
original `#UD` diagnostic and pre-instruction CPU state; it neither publishes
producer effects nor reaches a handler.

The retained `core-machine-vm86-delivery-smoke` continues to prove VM86
vector-6/TSS-stack delivery. `core-machine-exception-delivery-s2-smoke`,
`core-machine-80386-paging-smoke`, `core-machine-fpu-interface-s65-smoke`, and
`core-machine-bound-s54-smoke` retain active `#DE/#PF/#MF/#NM/#BR` routes; the
paging smoke continues to prove its four-dword `#PF` error-code frame.

## Similar-Issue Sweep

The exact production query was:

```text
rg -n "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode\\(" src/core/machine --glob '*.c' --glob '*.h'
```

It found 190 textual hits, all in `cpu_instructions.c` plus the exception
constant declaration. They classify into direct control/segment/prefix
`_SetExcept_UD` producers and `UndefinedOpcode` callers for unsupported
primary, `0F`, ModRM, profile, operand-form, and LOCK forms. They all converge
only through `ExecFinal`; no producer-specific delivery repair is needed.
`VCPUINS_EXCEPT_FPU_UNSUPPORTED` is a distinct provider boundary, not a
vector-6 producer. The second query,

```text
rg -n "_e_except_n\(" src/core/machine/cpu_instructions.c
```

confirmed that `ExecFinal` is the fault-delivery caller and that external
interrupt/software paths keep their existing serializer calls. The error-code
classifier is therefore constrained to `_e_except_n`'s exception path.

## Transfer

Real-mode vector-6 IVT delivery and migration of 41 retained no-handler smoke
contracts are a separate High TODO. Debug/breakpoint semantics, task state,
VME/PVI, reset policy, and wider trap policy remain the named 80386DX task,
debug, and VM86 boundary; no T326 evidence claims them.
