# T329 Task-Switch State-Machine Record

## Boundary

This supporting record constrains the active T329 task-system package. Intel
80386 behavior remains the authority. It does not allocate a task, replace the
active [T329 S3 packet](../../STATUS.md), or claim that later task, LDT, VM86,
or debug behavior is complete.

The record addresses a recurring construction risk: a task switch is not a
sequence of independently safe assignments. It is one architectural transition
whose validation, externally visible writes, loaded CPU state, and next
instruction must agree. A local smoke can expose a symptom, but cannot define
the transition model by itself.

## Task-Transition State Machine

S2 establishes, and S3 strengthens, the protected CPL0 direct far `JMP` from a
valid busy 32-bit TSS to an available 32-bit TSS. S4 adds direct far `CALL`
and GDT task-gate entry; S5 adds backlink-driven nested `IRET` return and IDT
task-gate entry to that same transition. Paging is disabled and the incoming
LDTR is null. Direct and memory-indirect transfer encodings are forms of one
transition, not separate state models.

| Phase | Required work | May publish architectural state? |
| --- | --- | --- |
| Decode | Classify profile, prefixes, `LOCK`, direct/indirect source, and selector. | No. |
| Preflight | Read old and new descriptors; verify old TR, target TSS type/presence/limit, every read/write span, target TSS image, null LDTR, CR3 boundary, CS/SS/DS/ES/FS/GS selectors and caches, target instruction byte, and target stack boundary. | No. |
| Plan | Materialize the outgoing TSS image, incoming CPU/cache image, old/new descriptor updates, and transition kind as private values. All field offsets are named and compile-time checked. | No. |
| Commit | Write the outgoing image; for nested CALL/task-gate entry, write backlink and retain old busy; for a backlink return, clear the outgoing busy state and retain the returning busy target; otherwise clear old busy. Then publish CR3, registers, EFLAGS including `NT` only when nested, segment caches, LDTR, TR, and `CR0.TS` in one non-faulting local sequence. | Yes. |
| Post-commit | Execute the target's next instruction or accept a pending IRQ according to the already-preflighted target state. | Yes. |

No operation that can introduce an ordinary architectural fault is allowed
after the first commit write. If the core cannot preflight that operation with
the same semantics, S2 stops and transfers the missing transaction boundary;
it does not retain a partial commit and add a local exception workaround.

## Required Direct-JMP Checkpoints

The owner smoke uses bounded execution budgets and records distinct outcomes:

1. source instruction decoded and preflight failure, with source CPU/TSS/GDT
   state retained as the declared fault contract;
2. successful commit, before any target-body side effect;
3. first target instruction; and
4. pending IRQ accepted after a successful incoming-IF transition.

S3 installs `#TS`, `#GP`, and `#SS` handlers on a valid source stack. It reads
the still-accessible old TSS and GDT after handler entry, proving that each
admitted descriptor/TSS/stack rejection occurred before the first commit
write. This supplements, rather than replaces, the no-IDT terminal boundary.

## TSS32 Image Contract

S2 names the 32-bit TSS fields instead of relying on an incidental C layout;
S3 binds every name to a shared offset constant and C11 `offsetof` assertion:
CR3, EIP, EFLAGS, EAX/ECX/EDX/EBX/ESP/EBP/ESI/EDI, followed by four-byte
selector slots for ES/CS/SS/DS/FS/GS/LDTR. The implementation asserts each
offset used by S2. It records the exact outgoing instruction boundary and
preserves the architectural width of each loaded field. Reserved slot bytes
are neither used as an alternate state source nor silently treated as a new
interface.

## Deferred T329 Progression

The later S boundaries are intentional dependency cuts, not omitted tests:

| Later S | Required architecture closure | Dependency on S2 |
| --- | --- | --- |
| S3 | Full 32-bit task image, CR3/cache details, and complete direct-JMP fault order. | Uses S2 preflight/commit framework. |
| S4 | Task gates, far `CALL` to TSS, backlink, and NT entry. | Requires one correct direct-switch state transition. |
| S5 | Nested-task `IRET`, bounded IDT task-gate and double-fault task chains. | Closed: requires and proves S4's backlink/NT semantics; arbitrary chains and failed-`#DF` reset policy transfer. |
| S6 | Closed non-null LDTR/LDT task images and task-local `TI=1` selector resolution for 16/32-bit transitions. | Uses S2's null-LDTR boundary and preserves its preflight/commit separation. |
| S7 | Task switching with paging and debug state. | Requires S3 plus the existing paging/debug boundaries. |

Bochs and PCjs may later be used only as bounded, read-only differential
diagnostic references under the source/research and recorder policies. They
cannot become product source, a runtime dependency, or an acceptance oracle.
