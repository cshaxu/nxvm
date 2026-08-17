# T391 S7: C1 Form And Context Ledger

`M5:T391:S7:C1-LEDGER:OK`

## Frozen batch and retained boundary

This ledger consumes exactly the 44 normalized C1 aggregates from one fresh,
contained owner-managed replay after the accepted C0A endpoint. It retains no
raw trace, address, PC, instruction byte stream, asset identity, path, digest,
firmware content or media content. The run stopped at its two-million-successful-
retirement containment limit: 1,981,742 successful C1 records, all classified,
zero source-unallocated records, no status or fault terminal, and no observed
C1 boot-transfer boolean. The limit is containment only, not startup coverage.

Every entry has real-address CPL0 context: PM, VM86, OS32, AS32, LOCK and REP
prefix are zero. Escape is `FF` throughout. `-` represents the capture's
non-applicable group/extension or opaque Core source-form sentinel. The retained
control key is `control/next/repeat-phase`; source/origin is a classifier key,
not an Intel citation. Aggregate emitted ticks are intentionally not retained:
this is a startup-semantic ledger, not a timing source ledger.

## Sufficiency and complete disposition

The aggregate identifies opcode family, selected group/ModR/M context where
available, control outcome and mode, but does not retain the full operands,
effective-address facts, immediate data or port, condition-code inputs, or a
finite semantic endpoint. It is therefore insufficient for an exact Intel row
or for a CPU timing repair. Every unit is **Deferred/nonphysical** to the same
receiver: **T391 task-closure startup-semantic decision**. That receiver must
record C1 as not admissible for the retained corpus and transfer any future C1
exact-row/context work only after a newly approved finite semantic checkpoint.
No row becomes physical-eligible merely because it has a classifier source
form or a repeated aggregate count.

| ID | Form / op / group | ModR/M / ext | Control / next / phase | Core source / origin | Successes | Sufficiency / disposition |
| --- | --- | --- | --- | --- | ---: | --- |
| C1-001 | other / `24` / - | 0 / - | 0 / - / 0 | - / primary | 8 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-002 | jmp-direct / `EB` / - | 0 / - | 2 / 2 / 0 | F73 / control-stack | 10 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-003 | out-immediate / `E6` / - | 0 / - | 0 / - / 0 | F26 / string-I/O | 23 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-004 | mov-immediate / `B0` / - | 0 / - | 0 / - / 0 | - / primary | 30 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-005 | in-immediate / `E4` / - | 0 / - | 0 / - / 0 | F24 / string-I/O | 19 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-006 | other / `F6` / 2 | 1 / 2 | 0 / - / 0 | - / primary | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-007 | mov-register-register / `8A` / - | 1 / 4 | 0 / - / 0 | - / primary | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-008 | mov-immediate / `B9` / - | 0 / - | 0 / - / 0 | - / primary | 18 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-009 | other / `3A` / - | 0 / - | 0 / - / 0 | - / primary | 4 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-010 | other / `74` / - | 0 / - | 1 / - / 0 | - / 80386-fallback | 3 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-011 | loop / `E2` / - | 0 / - | 2 / 2 / 0 | - / control-stack | 3 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-012 | other / `74` / - | 0 / - | 2 / 2 / 0 | - / 80386-fallback | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-013 | mov-immediate / `BE` / - | 0 / - | 0 / - / 0 | - / primary | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-014 | xor-register-register / `33` / - | 0 / - | 0 / - / 0 | - / primary | 17 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-015 | mov-immediate / `B8` / - | 0 / - | 0 / - / 0 | - / primary | 2 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-016 | mov-sreg / `8E` / - | 1 / 0 | 0 / - / 0 | F9 / 80386-fallback | 4 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-017 | sahf / `9E` / - | 0 / - | 0 / - / 0 | F7 / 80386-fallback | 198332 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-018 | other / `D1` / 2 | 0 / - | 0 / - / 0 | F4 / 80386-fallback | 263849 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-019 | other / `AB` / - | 0 / - | 0 / - / 1 | - / string-I/O | 65536 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-020 | loop / `E2` / - | 0 / - | 2 / 3 / 0 | - / control-stack | 65532 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-021 | loop / `E2` / - | 0 / - | 1 / - / 0 | - / control-stack | 4 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-022 | cmp-register-immediate / `80` / 0 | 1 / 0 | 0 / - / 0 | - / primary | 7 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-023 | other / `3B` / - | 0 / - | 0 / - / 0 | - / primary | 7 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-024 | other / `72` / - | 0 / - | 2 / 2 / 0 | - / 80386-fallback | 13 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-025 | other / `72` / - | 0 / - | 1 / - / 0 | - / 80386-fallback | 6 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-026 | other / `0C` / - | 0 / - | 0 / - / 0 | - / primary | 2 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-027 | other / `87` / - | 0 / - | 0 / - / 0 | - / primary | 3 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-028 | mov-immediate / `BB` / - | 0 / - | 0 / - / 0 | - / primary | 2 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-029 | mov-register-rm / `8B` / - | 1 / 0 | 0 / - / 0 | - / primary | 2 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-030 | mov-sreg / `8E` / - | 1 / 3 | 0 / - / 0 | F9 / 80386-fallback | 4 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-031 | other / `9F` / - | 0 / - | 0 / - / 0 | F8 / 80386-fallback | 198328 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-032 | lods / `AC` / - | 0 / - | 0 / - / 1 | - / string-I/O | 198313 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-033 | inc-register / `46` / - | 0 / - | 0 / - / 0 | - / primary | 594951 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-034 | xor-register-register / `32` / - | 0 / - | 0 / - / 0 | - / primary | 198313 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-035 | other / `E1` / - | 0 / - | 2 / 1 / 0 | - / control-stack | 198300 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-036 | other / `E1` / - | 0 / - | 1 / - / 0 | - / control-stack | 12 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-037 | jcc / `75` / - | 0 / - | 1 / - / 0 | - / 80386-fallback | 24 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-038 | test-accumulator-immediate / `A8` / - | 0 / - | 0 / - / 0 | - / primary | 12 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-039 | mov-immediate / `B1` / - | 0 / - | 0 / - / 0 | - / primary | 15 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-040 | other / `D3` / 2 | 0 / - | 0 / - / 0 | F5 / 80386-fallback | 15 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-041 | other / `83` / 7 | 1 / 7 | 0 / - / 0 | - / primary | 12 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-042 | dec-register / `4D` / - | 0 / - | 0 / - / 0 | - / primary | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-043 | other / `7B` / - | 0 / - | 1 / - / 0 | - / 80386-fallback | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |
| C1-044 | other / `E9` / - | 0 / - | 2 / 2 / 0 | F73 / control-stack | 1 | Insufficient; Deferred/nonphysical - T391 task-closure startup-semantic decision |

## Reconciliation and transfer

The count sum is 1,981,742, exactly equal to the capture terminal count and
classified count. All 44/44 aggregate units have the one stated disposition;
there are no accepted, non-applicable or blocked units. This resolves the
C1 aggregate as a complete batch without promoting any classifier field to
source authority. The bounded absence of the named boot-transfer checkpoint
means C1 is not an admissible next physical-retirement corpus for this retained
replay. The next receiver is the T391 task-closure startup-semantic decision,
which must transfer physical qualification as blocked on a new finite semantic
checkpoint or a separately identified functional defect.