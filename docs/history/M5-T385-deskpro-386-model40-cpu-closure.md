# M5 T385: DeskPro Model-40 80386 CPU Closure

## Task Record

T385 is the CPU-only successor to T384's selected 1986 original Compaq
DeskPro 386 Model 40 / DeskPro 386/16 audit.  It reconciles the 80386DX-16
architecture/state contract with the closed shared 80386DX source ledger. It
does not construct a DeskPro machine, repair devices or make a timing/L3
claim.

## S1: Shared CPU Ledger Reconciliation

S1 confirms that the current graph retains one 80386 CPU profile, execution,
translation, delivery and successful-retirement construction.  T340--T342's
Intel-constrained form/state evidence applies unchanged to the Model-40 CPU
semantic boundary; the Compaq 16 MHz selection does not require a duplicate
CPU path.  A clean current-source build and all 251 registered current-gate
tests pass.

The detailed [S1 evidence](../etc/evidence/t385-s1-deskpro-80386-cpu-closure.md)
records the exact CPU rows, current owners, full-gate result, the unrelated
non-current request-bridge build debt and the functional/timing transfers.
No source repair was needed.

## Closure

Coordinator review accepted S1 against the owner-approved Model-40 selection,
the T340--T342 80386DX closure ledger, actual current source and the T385 exit
criteria. The review confirms that 16 MHz is retained as a board-level clock
fact, not a duplicate CPU contract, and that every CPU-semantic row either
remains in the shared owner or is explicitly outside the selected CPU scope.
The clean current-source current gate passed 251/251.

T385 closes without a source repair. It transfers the first runnable Compaq
composition, selected device completeness and Compaq EGA personality to the
following DeskPro functional candidate; board/device timing stays with its
following candidate. The final DeskPro audit alone may make an L3 decision.
The task-level review is retained in [S1 closure evidence](../etc/evidence/t385-s1-task-closure-audit.md).
