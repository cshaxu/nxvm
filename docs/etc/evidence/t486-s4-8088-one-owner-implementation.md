# T486 S4 8088 One-Owner Implementation

`M5:T486:S4:8088-ONE-OWNER-IMPLEMENTATION:ACCEPTANCE-CANDIDATE`

The successful 8088 selector remains `string/I-O -> primary -> control/stack
-> compatibility`. This S adds no selector, public contract, machine state or
profile path.

| List-2 batch | Result | Proof |
| --- | --- | --- |
| MOV segment register; LDS/LES; XLAT | Primary now selects the exact Table-2-21 base, EA and four-clocks-per-transfer terms. | Retirement cases cover register/memory segment MOV, LDS and XLAT. |
| Group-2 | Primary now selects the F3 register/memory, one/CL count grammar and its documented two-transfer memory term. | Retirement case covers memory count-one; the existing decoder supplies the shared ModRM/CL facts. |
| PUSH/POP segment register | Control/stack owns their distinct 8088 source result; no stack path moved. | Retirement cases prove PUSH DS, POP DS and 8088 POP CS. |
| LOCK | The existing post-selection term now applies to 8088 after a successful source row. | LOCK ADD-memory retirement proves the base row plus two clocks. |
| ESC | The primary owner now selects Intel's exact CPU-side register/memory ESC cost; it does not include x87 service. | Retirement case proves register ESC. |
| Group-3; WAIT; non-string REP | No timing is invented. Compatibility continues to publish source-unallocated status for these unresolved/range/external cases. | Retirement cases prove MUL and WAIT are source-unallocated; the existing REP NOP case remains source-unallocated. |

The focused `core-machine-retirement-observation-s3-smoke` passes, and the
isolated Debug current specialized gate passes (356 targets). The implementation changes
three tracked code/test paths: 111 added and one removed line by `git diff
--numstat` before evidence. The retained production route is the existing
private Core timing chain; no legacy evaluator was reattached and no duplicate
state or compatibility implementation was introduced.
