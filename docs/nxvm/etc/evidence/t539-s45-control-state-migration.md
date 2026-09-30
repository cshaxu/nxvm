# T539 S45 Control-state migration map

S45 retires the two mixed CLTS/MSW receivers and only the retained
control-state block from `core_machine_descriptor_system_smoke.c`.  It does
not change CPU implementation, public ABI, machine profiles, firmware, or
assets.

| Original case family | Single receiving owner | Boundary |
| --- | --- | --- |
| CLTS on 80286/80386; TS and flags | `cpu-control-state-smoke` | CPU-local |
| CLTS 66/67 attributes and unsupported legacy profiles | `cpu-control-state-smoke` | CPU-local |
| CLTS CPL/VM86 rollback | `cpu-control-state-smoke` | CPU-local |
| CLTS LOCK rejection and real PIC IRQ delivery | `machine-control-state-board-smoke` | Public Core/PIC |
| SMSW/LMSW register and memory forms; CR0 PE/TS effects | `cpu-control-state-smoke` | CPU-local |
| SMSW/LMSW operand/address attributes and legacy rejection | `cpu-control-state-smoke` | CPU-local |
| SMSW/LMSW CPL/VM86/source-limit rollback | `cpu-control-state-smoke` | CPU-local |
| SMSW/LMSW LOCK rejection and IRQ delivery | `machine-control-state-board-smoke` | Public Core/PIC |
| MOV CR0/CR2/CR3 reads, writes, invalid forms and rollback | `cpu-control-state-smoke` | CPU-local |
| Early-80386 ignored-ModRM MOV CR compatibility | `machine-control-state-board-smoke` | Public machine configuration |

The board receiver owns only observations that require the real machine:
PIC delivery, public physical-memory reads of the interrupt frame, and the
existing board compatibility option.  Instruction value, prefix, privilege,
VM86 and synthetic cache-boundary semantics remain in the `x86-cpu` receiver.
Neither receiver has a second production execution path.

The early-80386 read retains the original `EAX == 0000000c` expectation: the
66-prefixed MOV reads a 32-bit general-register destination.  The migrated
test executes the existing implementation through the public machine contract
and does not alter that implementation.

Focused receiver runs passed on both available target toolchains:

```
M5:T539:S45:CONTROL-STATE:OK
M5:T539:S45:CONTROL-STATE-BOARD:OK
```

The T317 strict-owner audit and T332 fixture-lifecycle audit pass with 35
strict CPU owners.  The count intentionally falls by one: the two old strict
mixed targets become one CPU-local strict target, while the new public board
receiver is not a direct `x86-cpu` compile owner.  T344 historical fixture
shapes also passes unchanged.

The similar-case sweep covers every supported CPU-profile category represented
by the original tests: 8086/80186/80286/80386 legality, operand and address
attributes, LOCK, CPL/VM86, CR0/CR2/CR3, memory boundaries and PIC delivery.

The repository-only unit corpus passes 416/416 on both x64 and x86.  The x86
run uses the same configured source tree and MinGW toolchain through a
Makefiles generator because this host's Ninja ABI probe does not complete;
the direct `ctest -L unit -j8` corpus run completes in 175.81 seconds.  This
is generator-only verification plumbing, not a second source or test path.
