# T329 S4: Task-Gate and Far-CALL TSS Entry

## Scope

S4 extends the accepted protected task-switch transition with the two Intel
80386 entry causes that create a nested task: direct far `CALL` to an available
TSS and far transfer through a GDT task gate.  It covers the retained 16-bit
TSS image and the 80386 32-bit TSS image, with paging disabled and a null
incoming LDTR.  Nested-task `IRET`, IDT task gates, double fault, non-null
LDT, paging, and debug state remain outside this slice.

## Transition Contract

The direct-JMP planner/commit path now takes a private `nested` transition
property.  Preflight includes the target backlink write span.  The commit
then saves the outgoing image, writes the target backlink, retains the old
TSS busy descriptor, sets the target busy descriptor, publishes the incoming
image, and sets `EFLAGS.NT`.  A non-nested transfer clears the old busy bit
and does not publish backlink or `NT`.  Task-gate decoding validates the gate
selector/DPL/presence and delegates only its resolved TSS selector to this
same transition boundary; it has no independent task-state write path.

## Requirement-to-Proof Matrix

| Requirement | Focused evidence |
| --- | --- |
| 16-bit and 32-bit nested entry | 80286/80386 16-bit TSS and 80386 32-bit TSS direct far `CALL` execute to target HLT.  The 32-bit operand-size form is also executed. |
| CALL encodings | Immediate far `9A` and memory-indirect `FF /3` forms switch to the same target image. |
| Backlink, `NT`, busy state | Nested vectors read target offset `00h` as old `TR`, retain old busy, set target busy, and observe `NT`; task-gate JMP proves the non-nested alternative does not inherit those publications. |
| Task-gate route | GDT task-gate far CALL and JMP resolve to the target TSS; DPL rejection reaches the installed `#GP` handler before state commit. |
| Failure ordering | Existing installed-handler `#TS/#GP/#SS` direct-CALL vectors retain old TSS image and descriptors before the first commit write.  A not-present task gate reproducibly produces `#NP(38)` followed by the current contributory `#DF` terminal boundary; source `TR` and general registers remain the source task state. |
| Attributes and IRQ | Existing direct-TSS `66h`/`67h` vectors remain green.  Direct `LOCK 9A` and local `LOCK FF /3` rejection prove neither CALL form publishes source/TSS state; a nested 32-bit CALL with incoming IF proves IRQ acceptance only after transition commit. |

The owner smoke emits `M5:T329:S4:TSS-CALL-GATE:OK` and remains the existing
`current.core-machine-task-switch-smoke` target.  No public ABI, exception
framework, paging, provider, or product-visible interface changed.

## Transfer

S5 owns nested `IRET`, task return, IDT task gates, and double-fault chains.
S6 owns non-null LDT task images; S7 owns task paging and debug state.
