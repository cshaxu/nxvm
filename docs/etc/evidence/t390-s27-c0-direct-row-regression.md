# T390 S27: M40-C0 Direct-Row Regression

`M5:T390:S27:C0-DIRECT-ROW-REGRESSION:OK`

## Scope and fixed replay result

S27 consumes the complete corrected S26 C0 batch. The owner-authorized local
replay reaches `protected-return-c0` with 18,255 classified successful
retirements, zero source-unallocated successes, and 82 aggregation children
over the unchanged 75-parent universe. It retains no firmware, media, path,
digest, byte, PC, trace, or provenance data.

The corrected lexical-length control capture refines the former C0-024 LOOP
parent into two children: 289 taken successes with a two-component next
lexeme, and four fall-through successes. This is the only S26-to-S27 child
cardinality change; their sum is the retained 293-parent total.

## Complete child disposition

The 82 children are the replay rows in retained C0 order. Each is an exact
selected Intel 80386 PRM row already owned by one private Core classifier and
directly exercised by its listed project-owned regression. The table groups
only identical owner/regression dispositions; the `Children` column is the
complete one-based replay-row set, so no child is omitted.

| Children | Exact-row owner and Intel-primary ledger | Direct project-owned regression | Disposition |
| --- | --- | --- | --- |
| 1, 13, 23--24, 28, 40, 44, 46--47, 67, 72, 82 | Control/stack: direct and indirect transfers, Jcc/LOOP outcomes and lexical `m`; [T359 S3 ledger](t359-s3-four-profile-control-stack-source-ledger.md) | `core-machine-t359-s3-timing-smoke` | Accepted source row and direct regression; nonphysical |
| 2, 4, 7, 11, 14--19, 21, 27, 30--33, 35--39, 41--43, 45, 48--49, 54, 58--66, 70, 73, 75--76, 80 | Primary data/ALU, MOV, TEST, INC/DEC and group forms; [T359 S2 ledger](t359-s2-four-profile-arithmetic-data-source-ledger.md) | `core-machine-t359-s2-timing-smoke` | Accepted source row and direct regression; nonphysical |
| 3, 10, 20, 22, 34, 52--53, 77 | String and ordinary I/O, including repeat phase; [T359 S4 ledger](t359-s4-four-profile-string-io-source-ledger.md) | `core-machine-t359-s4-timing-smoke` | Accepted source row and direct regression; nonphysical |
| 8, 68--69, 71, 79, 81 | LMSW and control-register forms; [T359 S6 ledger](t359-s6-privileged-source-ledger.md) | `core-machine-t359-s6-timing-smoke` | Accepted source row and direct regression; nonphysical |
| 5--6, 9, 12, 25--26, 29, 50--51, 55--57, 74, 78 | 80386 fallback rows selected by the existing exact form owners, including CLD, CLI, SAHF, MOV Sreg and SAL; [T357 S3 ledger](t357-s3-80386-instruction-timing-ledger.md) and T390 S6--S12/S15 evidence | `core-machine-instruction-timing-ledger-smoke` plus the retained focused T390 owner smokes | Accepted source row and direct regression; nonphysical |

The grouped set contains 82 distinct children exactly once. The local replay
also verifies that every child is classified; this S does not use emitted ticks
as a source citation.

## Publication boundary and transfer

An accepted source/regression disposition is not a physical-retirement
qualification. The existing physical-contract rejection remains in force for
the whole C0 batch: no child is physical-eligible, no profile chooses physical
CPU time, and no device, board, C1, or L3 claim follows.

The next T390 receiver is therefore the physical-contract selection proof over
this complete source-and-regression-qualified C0 batch, or an earlier receiver
if that proof finds a new prerequisite. It must preserve the whole-batch
nonpublication boundary and cannot derive a board-clock ratio from these Core
rows.
