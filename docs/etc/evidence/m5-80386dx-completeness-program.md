# M5 80386DX Architecture-Completeness Program

## Decision And Boundary

The owner has selected Intel 80386DX CPU architecture completeness as a
pre-M6 M5 program. It replaces the former absence-of-consumer rationale for
withdrawing an 80386 family in this program. This is an architectural CPU
scope decision, not a current product compatibility claim, a Windows 3.1
acceptance claim, a timing-fidelity claim, or permission to broaden M5 into
whole-PC emulation.

The Intel 80386 Programmer's Reference Manual is the behavioral authority.
Existing form audits, including
[T309's audit](t309-80386-form-audit.md), are baseline evidence rather than a
claim that the listed bounded work is complete. Bochs and PCjs may be used as
read-only differential or design references under the
[source and research policy](../operations/policy/source-policy.md). They do
not replace the Intel authority, authorize source import, or establish a
compatibility claim.

## 80387 Boundary

This program includes the 80386-side behavior that controls or reports an
external coprocessor: CR0 `MP`, `EM`, and `TS`; `WAIT`/`FWAIT`; ESC handling;
`#NM`; task-switch `TS` behavior; and architected external coprocessor-fault
delivery at the CPU boundary. The exact admitted behavior must also state how
the configured no-coprocessor or existing FPU boundary is observed.

It excludes 8087, 80287, and 80387 execution: numeric formats, arithmetic,
environment/state implementation, coprocessor-internal exception processing,
and an x87 instruction-completeness claim. Those remain a separate future
project and are not silently admitted by this program.

## Continuous Delivery Shape

The ordered candidates in [QUEUE.md](../../QUEUE.md) are intentionally
unnumbered. A package advances one admitted task at a time under
[Execution Policy](../../rules/EXECUTION.md); it does not reserve numeric `T`
identifiers or create several active packets.

| Family package | Expected bounded tasks | Completion evidence |
| --- | ---: | --- |
| Ordinary execution, operands, flags, strings, and control | 5--7 | Form/flag/operand focused probes and retained corpus results. |
| Exceptions, interrupts, returns, control registers, and x87 coupling | 3--5 | Producer, frame, priority, restart, and control-boundary probes. |
| Segmentation, descriptors, privilege, and gates | 3--5 | Descriptor-classification and transfer/return probes. |
| Non-PAE paging, protection, invalidation, and diagnostics | 3--4 | Translation, permission, fault, and invalidation probes. |
| Tasks, LDT, virtual-8086, debug registers, and test registers | 4--6 | State-transition, privilege, and register-form probes. |
| Architecture coverage and remediation closure | 2--5 | Form-to-evidence matrix, retained corpus, and package-close audit. |

The program is therefore expected to require about **25--40 independently
bounded implementation tasks**. Exact task subdivision follows the source and
test seams discovered at admission; it is not a commitment to a fixed task
count. The final coverage matrix must classify every in-scope architectural
form and behavior as implemented-and-tested, outside the 80386 architecture,
or an explicit external-coprocessor boundary. It must not leave a form merely
"not needed by Windows" or "without a product consumer."

## Reuse, Abstraction, And Source Discipline

Before admitting each instruction or system family, the task review records:

- existing decode, operand, flag, memory, privilege, exception, and state
  helpers that could correctly own the behavior;
- every caller of a candidate shared helper and the focused tests that cover
  those callers; and
- whether a new abstraction has at least two concrete callers and one stable
  responsibility, rather than merely hiding a repeated line of code.

If caller coverage is missing, add or confirm that coverage before changing a
shared helper. Do not extract speculative helpers or duplicate an existing
clear boundary. In an existing implementation file, preserve its established
structure, naming, and local coding style. File size alone is not a reason to
split behavior: further instruction implementations belong in
`cpu_instructions.c` when they retain that file's CPU-instruction ownership and
style. A split requires a real subsystem or ownership boundary under the
[Coding Rules](../../rules/CODING.md).

## Efficient, Non-Deferred Verification

Focused synthetic architecture probes are the primary construction evidence;
they may share fixtures and retained regressions so each bounded task does not
recreate a full-system scenario. This efficiency does not defer validation to
Windows setup and does not relax the Execution Policy: every admitted task
runs its focused evidence, applicable local invariants, and established corpus.
Shared-helper, decoder, fault-delivery, or public-boundary changes also run the
relevant broader regressions. A package closes only after its full coverage
matrix and required package-close audit pass.

Windows 3.x installation and boot remain later integration checkpoints. They
can expose gaps, but cannot substitute for form-level architectural evidence.

## Promotion And Retirement

This document supports the M5 roadmap and Queue decision. It is retired or
replaced when the M5 architecture-coverage closure audit records the final
80386DX disposition matrix, or when an owner-approved roadmap decision changes
the program boundary.
