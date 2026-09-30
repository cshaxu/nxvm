# M5 T539 S51 Control-Transfer Receiver Map

## Source families and final receivers

The final mixed source had six families. Each now has exactly one CPU-local
receiver. `cpu_control_transfer_far_smoke.c` links only `x86-cpu` and uses the
instruction fixture's explicit memory bus, segment caches and fault snapshot;
it does not construct a Core machine or touch a board-private field.

| Former family | Receiver | Reason |
| --- | --- | --- |
| Same-CPL `RETF` descriptor rejection | S51 CPU receiver | DPL/non-present selector rejection, descriptor access preservation and terminal fault rollback. |
| Immediate protected far JMP/CALL 16/32-bit forms | S51 CPU receiver | Exact 16/32-bit target, return-frame and resumed instruction behavior. |
| Indirect protected far JMP/CALL and invalid `/3`/`/5` forms | S51 CPU receiver | DS-relative far pointers, return behavior, #UD and atomic fault state. |
| Four-profile real-mode far JMP/CALL/RETF and boundary pointer | S51 CPU receiver | Segment/stack behavior, `RETF imm` and the `DS:FFFE` pointer boundary. |
| Four-profile real-mode near/indirect control residual | S51 CPU receiver | Direct, register/memory-indirect and immediate-return behavior across all profiles. |
| Legacy reserved `FF` encodings | S51 CPU receiver | All three retained reserved encodings preserve CPU state while issuing #UD. |

The original mixed source is deleted. S53 still owns its separate protected
far/data source and must migrate that source on its own merits; it is neither a
duplicate receiver nor S51 acceptance evidence.

## Build correction discovered by the complete build

The complete 32-bit build found two older protected-16 board callers that had
not followed the current four-argument
`test_protected_16_prepare_with_planar_parity()` test-helper contract.  Both
are explicitly protected-16 tests, so their missing `default32` argument is
`LIB_FALSE`.  This is a compile-closure correction only: it neither changes
the CPU implementation nor broadens S51's transfer behavior.

## Verification record

Both CPU-local focused targets emit the three retained historical/scope
markers on x64 and x86:

```
M5:T303:CONTROL-TRANSFER:OK
M5:T401:S23:FAR-RETURN-PROFILES:OK
M5:T539:S51:CPU-CONTROL-TRANSFER-FAR:OK
```

Fresh complete builds and repository-only unit runs pass 418/418 on each
width.  The x64 MinGW Makefiles tree and the fresh x86 MinGW Makefiles tree
also pass the current specialized-gate aggregate, including the 38-owner T332
fixture-lifecycle check and the 104-row T344 retained-fixture inventory.
Documentation governance and `git diff --check` pass.  The direct-private
sweep finds no active CMake or test reference to the retired source/target;
the remaining matches are historical evidence and therefore intentionally
retain their original names.

No production/API, Shared, firmware, asset, INI or executable input changed,
so no executable artifact is rebuilt for this test/CMake/documentation-only
delivery.
