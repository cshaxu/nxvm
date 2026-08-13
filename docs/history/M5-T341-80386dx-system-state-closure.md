# M5 T341: 80386DX System, VM86, Paging, And Debug Closure

## Task Record

T341 is the Queue-ordered successor to the closed 80386DX form package. It
owns the residual vertical state composition required for a truthful 80386DX
profile claim: privileged control and table state, VM86, task/TSS, paging, and
debug/breakpoint delivery. It preserves genuine 16-bit and 32-bit layouts and
reuses earlier evidence only after a state-owner comparison.

The active packet in [CURRENT](../states/CURRENT.md) owns each admitted S.
The linked proposal remains current until T341 closes.

## S1 State Allocation

S1 creates the complete state-owner ledger before implementation. It maps CR,
DR, TR/LDT/table, VM86, task/TSS, paging, and vector-1 boundaries through all
readers/writers and validation-to-commit paths. It may reuse accepted T320,
T321, T325, T326, T329, and T337 evidence only where their exact frame,
privilege, mode, and publication boundaries match the T341 row.

The indexed allocation ledger determines the later bounded mechanism S work.
It cannot close a runtime row or introduce a new execution path.
