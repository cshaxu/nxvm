# T390 S26: C0 Control-Context Corrective

## Root Cause And Shared Repair

S26 corrects a defect in accepted S25 before its transferred direct-row batch begins. The S25 control-outcome calculation used `t_cpuins_data.oplen`. That member is the 15-byte diagnostic capture window, not the decoded instruction length. A two-byte conditional branch could therefore compare its post-execution EIP with an incorrect fifteen-byte fall-through and be published as taken.

The shared retirement-observation context owner now uses the existing pure `core_machine_cpu_instruction_lexeme_scan()` over the decoder-captured bytes and the old code-size state. It computes fall-through from the scanner's decoded byte count. No public contract, CPU semantics, timing value, profile behavior, physical contract, or test-only production API changes. If the scanner cannot supply a successful control form, the observation stays `none`/unavailable rather than inferring a disposition.

The focused Core observer regression proves both parts of the boundary with two-byte short conditionals: reset-default `JNZ` is taken and publishes the target's one-component lexeme; reset-default `JZ` falls through and publishes no target lexeme. Existing reset, disabled-provider and physical-rejection assertions remain in the same owner smoke.

## Frozen-Batch Reconciliation

The affected S25 context receiver is only the 17-parent control outcome/lexeme class:

`C0-001`, `C0-012`, `C0-013`, `C0-023`, `C0-024`, `C0-025`, `C0-028`, `C0-038`, `C0-041`, `C0-043`, `C0-046`, `C0-050`, `C0-051`, `C0-052`, `C0-060`, `C0-065`, and `C0-075`.

| Reconciliation fact | Before S26 | After S26 |
| --- | ---: | ---: |
| Frozen parent universe | 75 | 75 |
| Successful classified retirements | 18,255 | 18,255 |
| Source-unallocated successful retirements | 0 | 0 |
| Refined context aggregates | 81 | 82 |
| S25 control-context state | Invalid for conditional fall-through | Corrected from decoded lexical length |
| Physical-eligible aggregates | 0 | 0 |

The additional child is a genuine normalized-context refinement: one previously monolithic loop parent has both taken and fall-through successes under its already frozen C0 semantic domain. The corrected replay also changes affected conditional outcome/target availability where the old diagnostic-window comparison was false. S25's ModR/M and repeat-phase facts are not affected.

Every affected child remains Deferred/nonphysical, together with every unchanged child, to the single **T390 C0 direct-row regression batch**. The batch is now 82 children over the unchanged 75 parents and 18,255 successes. No earlier S25 control outcome is an Intel-row selection or physical proof.

## Verification And Transfer

- `core-machine-retirement-observation-s3-smoke` passes its new taken/fall-through assertion and existing lifecycle/physical-boundary coverage.
- A fresh owner-authorized local replay reaches the frozen `protected-return-c0` terminal with 18,255 successful classified retirements, zero source-unallocated successes and 82 refined aggregates. No asset identity, path, hash, bytes, PC, trace, or provenance is retained here.
- The next T390 S must re-admit the complete 82-child direct-row regression batch. It must not reuse the invalid S25 outcome classification, select physical retirement, or resume first-terminal repair.

## Coordinator Acceptance

Coordinator actual-diff review accepts P1 `ce67d58d`. The repair uses the existing pure decoder lexeme scanner only at the shared Core observation owner, replaces no instruction semantics or time publisher, and leaves unavailable context non-inferential. The focused taken/fall-through regression proves the former diagnostic-window defect is closed; fresh scalar-only C0 replay preserves the 75-parent/18,255-success universe and exposes the correct 82-child refinement. The current-gates GCC build, full 284/284 CTest run, documentation gate, source-policy boundary and actual diff pass. S26 is accepted. All 82 children remain nonphysical and transfer as one C0 direct-row regression batch; no CPU physical qualification, C1, board/device timing or L3 conclusion is accepted.
