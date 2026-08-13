# T337 S1: Real-Mode Invalid-Opcode Delivery

## Scope and policy

T337 S1 closes only the real-mode `#UD` final-delivery gap left by T326.
Every `#UD` producer continues to converge at `ExecFinal`; a valid IVT entry
for vector 6 transfers through the existing real-mode serializer, and a failed
serializer attempt rolls back to the originating instruction and remains the
explicit terminal diagnostic boundary. This is not a firmware default-vector
policy and does not change protected or VM86 delivery.

## Mechanism proof

`core-machine-real-ud-delivery-s1-smoke` installs IVT vector 6 with an
`INC AX; HLT` handler, then exercises these independent producer classes:

| Producer class | Bytes and profile | Required observable |
| --- | --- | --- |
| Primary | `F1`, 80386 | restart frame and vector-6 handler transfer |
| `0F` escape | `0F FF`, 80386 | same common finalizer route |
| Operand form | `62 C0`, 80386 | register-form BOUND `#UD` reaches vector 6 |
| Profile gate | `66 90`, 80286 | rejected prefix has the same delivery contract |
| LOCK gate | `F0 90`, 80386 | prefix rejection occurs before producer publication |

For every delivered case, the owner verifies saved IP `0200h`, saved CS and
pre-fault FLAGS, 16-bit SP decrement by six with high-half retention,
interrupt-gate IF/TF clearing, vector entry at `0100h`, and subsequent
handler progress to `0102h`. The failed-IVT case constrains the IDTR below the
vector-6 entry and verifies the original `#UD` diagnostic, no delivered event,
and restored IP/SP/FLAGS/CS/SS state. Its terminal test deliberately leaves TF
clear: a pending single-step delivery is a distinct vector-1 event and is not
evidence about failed vector-6 atomicity.

## Current-gate owner inventory

`PROJECT_T337_UD_CURRENT_GATE_TARGETS` in `CMakeLists.txt` is the durable
mechanical upper-bound inventory. At configure time it proves all 69 listed
targets remain in `PROJECT_CURRENT_SMOKE_TARGETS`, each has an owned
`tests/machine` source containing a `#UD` producer/assertion token, and no
current-gate `tests/machine` source with such a token is omitted.

The list intentionally exceeds real-mode owners: several fixtures also use
protected-only or VM86 rejection vectors. This conservatively prevents a
mixed-mode owner from silently escaping the review. The new owner smoke is the
single explicit valid-IVT proof; a real-mode owner that deliberately omits an
IVT entry retains its terminal contract, while protected/VM86 owners retain
their existing delivery scope. Later form packages may install a vector-6
handler when their own contract needs it; they do not create opcode-local
exception paths.

## Source sweep and retained boundaries

The production sweep over `VCPUINS_EXCEPT_UD`, `_SetExcept_UD`, and
`UndefinedOpcode` finds direct primary, `0F`, operand, profile, and LOCK
producers only in `cpu_instructions.c`; their common mechanism owner is
`ExecFinal`. The finalizer now includes `VCPUINS_EXCEPT_UD` in the existing
real-mode delivery group used by `#DE/#PF/#MF`, preserving the separate
vector-specific mapping and the T331 rollback helper.

Debug/breakpoint vector semantics, NMI/IRQ ordering, task/reset behavior,
VME/PVI, and non-`#UD` shared delivery rows remain in later T337 subtasks.
