# T564 S11: X86 Core x87 and virtual-interrupt audit

## Selected-product facts

The four published PC Apps configure no coprocessor:

| App | CPU | FPU |
| --- | --- | --- |
| My5160 | 8088 | none |
| My5170 | 80286 | none |
| MyDeskPro386 | 80386 | none |
| NXVM | 80386 | none |

Consequently no current PC release claims numerical x87 execution.  The
published 486SX Core profile also requires `FPU_NONE`; it is a Core-selected
model, not an admitted PC-App configuration.

## Existing x87 boundary

`x86-core/cpu` owns `WAIT`/`ESC`, CR0 `MP`/`EM`/`TS` gates and `#NM`/`#MF`
delivery.  `x86-core/fpu` owns optional FPU state and the only finite numerical
subset: reset, a limited m32 load/store/control-word subset and selected 8087
register arithmetic.  The 8087 subset is not a full numerical/FPU environment
implementation.  80287 and 80387 configurations retain the established
external-command handoff: they do not silently execute an invented numeric
model.

CPU/FPU compatibility is enforced once by CPU execution: 8087 is admitted for
8086-semantics/80186 CPUs, 80287 for 80286/80386 and 80387 for 80386.  The
Machine rejects an integrated FPU for 486SX and any FPU for V30.  Existing
time values are labelled External-L2 and advance through the sole existing Core
deadline; this audit makes no silicon timing claim.

## Virtual interrupts

VME is a single CPU-local condition, not a general virtual-interrupt service:
the selected 486SX profile, protected VM86 execution, `IOPL < 3` and CR4.VME
must all be active.  Existing CPU code owns VIF/VIP handling for PUSHF/POPF,
CLI/STI, IRET, software INT and pending delivery; it uses the ordinary
exception/delivery path rather than a second interrupt queue.  Current product
profiles do not select this 486SX capability.

## Verification and disposition

Sixteen owner-local FPU, VME, VM86, Machine-profile and board-composition tests
passed on both x64 and x86 Release Ninja trees.  They cover no-FPU ESC forms,
selected 8087 behavior, unsupported 80287/80387 handoff, WAIT/exception
conditions, FPU profile gates, VME flags/control/INT/IRET routes and VM86
delivery.  No production, test, ABI, manifest, binary, INI or media change was
needed.

The complete x87 numeric-engine task is recorded as a future Queue candidate.
It requires a separately admitted personality, finite source-backed corpus,
exception/timing contract and any product wiring.  This audit does not claim
that all x87 forms, formats, environments or virtual-interrupt extensions are
implemented.
