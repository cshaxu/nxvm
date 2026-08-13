# T338 S5: 8086/80186 Profile-Close Audit

## Allocation Reconciliation

| S1 allocation | Accepted T338 proof | Closure disposition |
| --- | --- | --- |
| ALU, adjustment, conversion, Group 1--3, shifts/rotates, conditions, loops, reserved primaries, `F6/F7`, `FE` | [S2 matrix](t338-s2-legacy-alu-profile-matrix.md) | Closed for 8086/80186 defined state and rejection boundaries. |
| Data movement, XCHG, segment/REP prefixes, strings, stacks, real control/interrupt, HLT, ordinary I/O, LES/LDS, `FF /2`--`/6` | [S3 matrix](t338-s3-legacy-data-control-profile-matrix.md) | Closed for real 16-bit 8086/80186 behavior. |
| `60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9` | [S4 matrix](t338-s4-80186-extension-profile-matrix.md) | Closed for 8086 rejection and 80186 acceptance. |
| `F0` legacy prefix | T328 retained `core-machine-legacy-lock-s1-smoke` evidence | One shared rule: valid 8086/80186 bus-prefix transparency; it never legalizes invalid forms. |
| Protected selectors/gates/IOPL and 80286 table/transfer behavior | Queue 80286 descriptor-transfer and 80286 LOCK/profile candidates | Transferred; not a T338 low-profile claim. |
| 80386 operand/address width, system state, VM86, paging, debug | Queue 80386DX width/integer and 80386DX system-state candidates | Transferred; not a T338 low-profile claim. |
| WAIT/ESC CPU boundary and x87 arithmetic/state | T316 S65 retained external-coprocessor evidence | CPU-side boundary retained; x87 execution remains external. |

## Mechanical Review

The audit compared S1's complete primary-form table, S2--S4 evidence, the
named CMake current-gate owners, primary metadata/dispatch, T328's one LOCK
owner, Queue candidates, and TODO debt. Every T338-allocated 8086/80186 row
has an accepted owner; no later-profile route is used as its sole proof. The
only reproduced S4 profile defect was repaired across all four INS/OUTS
handlers before acceptance.

## Verification

Documentation governance, `git diff --check`, the configured artifact verifier,
and the complete current-gate suite are recorded in the S5 delivery. No runtime
or artifact source change is made by this audit.
