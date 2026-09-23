# T390 S23: M40-C0 Convergence Ledger

## Scope And Frozen Universe

This is the durable coverage ledger for the T390 **M40-C0: protected-return initialization** semantic stage. Its universe is one fresh, owner-managed Model-40 BYOB replay with the selected 80386DX-16 backbone, 1 MiB, no FPU, one 1.2 MB removable drive, no fixed disk and cold reset. The stage starts at the first successful retirement after reset and ends at the first real-address successful retirement following at least one protected-mode successful retirement. The stage contains no address, byte, ROM/media identity, asset path, hash, raw trace or firmware provenance.

The ledger unit is one normalized successful-retirement aggregate. Its complete context key is the form, operand form, primary/escape opcode, group selector, opaque Core form identifier, classifier origin, emitted tick count, CPL, protected/virtual mode, operand/address size, LOCK and REP state shown below. `FF` and `255` are the capture's non-applicable sentinels; `-` is the opaque Core form-identifier sentinel. Neither scalar identifies an Intel timing row. The emitted tick count is diagnostic classifier output, not a source citation or a basis for inference.

## Allowed Dispositions And Completion

- **Accepted:** direct exact Intel-primary row/context citation, project-owned focused regression, and the existing physical-boundary proof identify the unit; its proof owner is the Core timing owner.
- **Non-applicable:** documented reason proves the unit cannot enter the selected C0 physical candidate.
- **Deferred/nonphysical:** a named receiver owns the whole underdetermined unit or class; it remains rejected before physical elapsed-time publication.
- **Blocked:** a named authority or implementation prerequisite prevents sufficient capture or exact disposition.

All current entries are **Deferred/nonphysical - receiver: T390 C0 exact-row/context matrix batch**. This is intentional: S22 establishes classifier-family origin, not exact Intel-row/context proof. No entry is physical-eligible.

The C0 batch is complete only when every listed entry is either Accepted with its exact proof/regression or assigned a whole-class Non-applicable, Deferred/nonphysical or Blocked disposition with its named receiver. A later T390 S may consider a C1 admission only after this complete C0 batch is reconciled; a farther replay, new terminal or containment-budget result cannot replace that condition. T390 as a whole remains open until every separately admitted finite corpus stage is exhausted or explicitly transferred under the proposal's physical-qualification predicate.

## Reconciliation

Fresh normalized replay reconciliation: **75 aggregate units**, **18,255 successful classified retirements**, 75/75 units origin-attributed, 75/75 units classified, zero source-unallocated successes and no status/fault terminal. The stage is finite; its prior containment budget is not a completion condition.

## Current C0 Entries

| ID | Form | Operand | Op | Esc | Group | Core form | Origin | Ticks | CPL | PM | VM | OS32 | AS32 | LOCK | REP | Successes | Current disposition / receiver |
| --- | --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| C0-001 | jmp-far-direct | not-applicable | `EA` | `FF` | 255 | F76 | control/stack | 14 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-002 | mov-immediate | not-applicable | `B0` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 45 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-003 | out-immediate | not-applicable | `E6` | `FF` | 255 | F26 | String/I-O | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 47 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-004 | mov-immediate | not-applicable | `B4` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-005 | sahf | not-applicable | `9E` | `FF` | 255 | F7 | 80386 fallback | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-006 | cli | not-applicable | `FA` | `FF` | 255 | F6 | 80386 fallback | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-007 | mov-immediate | not-applicable | `B8` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 8 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-008 | lmsw-register | not-applicable | `0F` | `01` | 255 | - | 80386 privileged | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-009 | mov-sreg | register | `8E` | `FF` | 255 | F9 | 80386 fallback | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-010 | in-immediate | not-applicable | `E4` | `FF` | 255 | F24 | String/I-O | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 8 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-011 | test-accumulator-immediate | not-applicable | `A8` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-012 | jcc | not-applicable | `75` | `FF` | 255 | - | 80386 fallback | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-013 | jmp-direct | not-applicable | `EB` | `FF` | 255 | F73 | control/stack | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 668 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-014 | mov-register-register | not-applicable | `8B` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-015 | cmp-register-immediate | not-applicable | `80` | `FF` | 7 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-016 | xor-register-register | not-applicable | `32` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-017 | xor-register-register | not-applicable | `33` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 262 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-018 | mov-immediate | not-applicable | `BE` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-019 | mov-immediate | not-applicable | `B9` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-020 | lods | not-applicable | `AC` | `FF` | 255 | - | String/I-O | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 58 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-021 | mov-register-register | not-applicable | `8A` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 61 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-022 | out-dx | not-applicable | `EE` | `FF` | 255 | F27 | String/I-O | 11 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 100 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-023 | loop | not-applicable | `E2` | `FF` | 255 | - | control/stack | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 10 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-024 | loop | not-applicable | `E2` | `FF` | 255 | - | control/stack | 13 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 293 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-025 | other | not-applicable | `76` | `FF` | 255 | - | 80386 fallback | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-026 | sal-register-one | not-applicable | `D0` | `FF` | 4 | F3 | 80386 fallback | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-027 | other | not-applicable | `8B` | `FF` | 255 | - | primary | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-028 | other | not-applicable | `FF` | `FF` | 4 | F74 | control/stack | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 5 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-029 | cld | not-applicable | `FC` | `FF` | 255 | F2 | 80386 fallback | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-030 | mov-immediate | not-applicable | `BB` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-031 | mov-register-rm | not-applicable | `8B` | `FF` | 255 | - | primary | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-032 | other | not-applicable | `F7` | `FF` | 0 | - | primary | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-033 | mov-immediate | not-applicable | `BA` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-034 | other | not-applicable | `EC` | `FF` | 255 | F25 | String/I-O | 13 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-035 | mov-immediate | not-applicable | `BF` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-036 | mov-register-rm | not-applicable | `8A` | `FF` | 255 | - | primary | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 6 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-037 | mov-immediate | not-applicable | `BD` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 8 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-038 | other | not-applicable | `E9` | `FF` | 255 | F73 | control/stack | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 7 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-039 | inc-register | not-applicable | `42` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 40 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-040 | other | not-applicable | `FE` | `FF` | 0 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 38 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-041 | jmp-direct | not-applicable | `EB` | `FF` | 255 | F73 | control/stack | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 42 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-042 | dec-register | not-applicable | `4A` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 42 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-043 | other | not-applicable | `FF` | `FF` | 4 | F74 | control/stack | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-044 | other | not-applicable | `83` | `FF` | 0 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-045 | other | not-applicable | `81` | `FF` | 7 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-046 | jcc | not-applicable | `75` | `FF` | 255 | - | 80386 fallback | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-047 | mov-sreg | memory | `8E` | `FF` | 255 | F10 | 80386 fallback | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-048 | other | not-applicable | `AB` | `FF` | 255 | - | String/I-O | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-049 | other | not-applicable | `AB` | `FF` | 255 | - | String/I-O | 5 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 16382 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-050 | other | not-applicable | `72` | `FF` | 255 | - | 80386 fallback | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-051 | jcc | not-applicable | `75` | `FF` | 255 | - | 80386 fallback | 8 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-052 | other | not-applicable | `72` | `FF` | 255 | - | 80386 fallback | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-053 | other | not-applicable | `3D` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-054 | other | not-applicable | `24` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-055 | other | not-applicable | `F6` | `FF` | 0 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-056 | other | not-applicable | `A1` | `FF` | 255 | - | primary | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-057 | other | not-applicable | `3C` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-058 | other | not-applicable | `0C` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-059 | mov-immediate | not-applicable | `B3` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-060 | other | not-applicable | `E9` | `FF` | 255 | F73 | control/stack | 12 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-061 | other | not-applicable | `0F` | `01` | 255 | - | 80386 privileged | 11 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-062 | other | not-applicable | `0F` | `20` | 255 | - | 80386 privileged | 6 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-063 | other | not-applicable | `0D` | `FF` | 255 | - | primary | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-064 | other | not-applicable | `0F` | `22` | 255 | - | 80386 privileged | 10 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-065 | other | not-applicable | `FF` | `FF` | 5 | F78 | control/stack | 33 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-066 | mov-immediate | not-applicable | `B8` | `FF` | 255 | - | primary | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-067 | mov-sreg | register | `8E` | `FF` | 255 | F9 | 80386 fallback | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 2 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-068 | mov-immediate | not-applicable | `B0` | `FF` | 255 | - | primary | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-069 | other | not-applicable | `88` | `FF` | 255 | - | primary | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-070 | out-immediate | not-applicable | `E6` | `FF` | 255 | F32 | String/I-O | 4 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-071 | cld | not-applicable | `FC` | `FF` | 255 | F2 | 80386 fallback | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 16 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-072 | other | not-applicable | `0F` | `20` | 255 | - | 80386 privileged | 6 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-073 | other | not-applicable | `25` | `FF` | 255 | - | primary | 2 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-074 | other | not-applicable | `0F` | `22` | 255 | - | 80386 privileged | 10 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |
| C0-075 | jmp-far-direct | not-applicable | `EA` | `FF` | 255 | F76 | control/stack | 17 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | Deferred/nonphysical - T390 C0 exact-row/context matrix batch |

## Universe Refinement Rule

A future change may refine this universe only with evidence that changes the admitted M40-C0 semantic domain. The coordinator must record the reason, the changed unit set and the before/after disposition of every affected entry before implementation continues. It may not replace this finite checkpoint with an unbounded boot path, a retirement budget or a newly observed first terminal.

## Transfer

The next receiver is one complete **T390 C0 exact-row/context matrix batch**: reconcile the 75 entries as a batch against Intel-primary authority, add project-owned regressions for any Accepted entries, and transfer each underdetermined class whole. Do not select individual repairs merely because a replay reaches a different terminal. This ledger does not qualify physical retirement, M40-C1 boot transfer, board/device timing or DeskPro L3.

## Coordinator Acceptance

Coordinator actual-diff review accepts P1 a7d9b1c1. The indexed evidence fixes one finite M40-C0 universe, retains all permitted normalized context without external asset data, and reconciles 75 aggregate units to 18,255 successful classified retirements. Each unit has an explicit nonphysical receiver; no tick value or opaque Core form identifier is treated as an Intel timing citation. Documentation governance, ledger cardinality/sum and artifact-boundary review pass. The next T390 work is one whole C0 exact-row/context matrix batch; physical retirement, C1, board/device timing and L3 remain unaccepted.
