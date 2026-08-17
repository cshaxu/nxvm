# T390 S25: M40-C0 Shared Context Capture

## Scope And Publication Boundary

S25 consumes the frozen T390 M40-C0 protected-return corpus and the S24 exact-row/context matrix. It does not alter instruction behavior, select an Intel timing value, publish physical elapsed time, or advance M40-C1, DeskPro board/device timing, or L3.

The shared Core retirement observer now copies only five bounded scalar facts at the post-decode, pre-callback publication boundary:

- ModR/M register-versus-memory form and the three-bit extension/selector, with an explicit unavailable sentinel;
- control outcome (`none`, fall-through, or taken) and, for a taken control transfer, the copied next-lexeme component count, again with an unavailable sentinel; and
- the repeat phase (`none`, primitive, zero-count, first iteration, or continuation).

The repeat phase is reset once per instruction-cost classification and is latched only by the existing string timing owner. It is not inferred from emitted ticks. The public record holds copied enums/scalars and no CPU, executor, RAM, firmware, media, or profile pointer. Existing observer-provider lifecycle and physical-contract ordering remain unchanged.

## Frozen-Batch Reconciliation

The S23 parent universe remains exactly 75 aggregate units and 18,255 successful classified retirements. S25 refines the **aggregation key**, not the admitted semantic domain: the newly available normalized context splits six parents into 81 child aggregates while preserving the same 18,255-success total and C0 protected-return terminal.

| S24 receiver class | Parent IDs | Before S25 | After S25 | Residual receiver |
| --- | --- | --- | --- | --- |
| Control outcome/lexeme | C0-001, C0-012, C0-013, C0-023, C0-024, C0-025, C0-028, C0-038, C0-041, C0-043, C0-046, C0-050, C0-051, C0-052, C0-060, C0-065, C0-075 | Blocked: missing outcome and/or next lexeme | Context-complete, Deferred/nonphysical | T390 C0 direct-row regression batch |
| Primary ModR/M shape | C0-027, C0-032, C0-040, C0-044, C0-045, C0-055, C0-069 | Blocked: missing register/memory shape | Context-complete, Deferred/nonphysical | T390 C0 direct-row regression batch |
| String repeat phase | C0-048, C0-049 | Blocked: missing first/continuation fact | Context-complete, Deferred/nonphysical | T390 C0 direct-row regression batch |
| Privileged selector/form | C0-061, C0-062, C0-064, C0-072, C0-074 | Blocked: missing ModR/M extension/shape | Context-complete, Deferred/nonphysical | T390 C0 direct-row regression batch |
| Already source-sufficient direct rows | Remaining 44 parents | Deferred/nonphysical | Unchanged Deferred/nonphysical | T390 C0 direct-row regression batch |

Thus no S24 Blocked receiver remains blocked **for lack of copied context**. All 75 parents are still nonphysical: 31 are newly context-complete and the original 44 remain direct-row-regression pending. This is not an exact-row acceptance and does not make any child physical-eligible.

## Verification

- The focused Core observer smoke checks reset/lifecycle behavior plus neutral values for all new fields on classified and source-unallocated paths.
- The focused aggregate smoke proves otherwise identical records remain distinct when the ModR/M selector differs, including the unavailable sentinel.
- A fresh owner-authorized local replay reaches `protected-return-c0` with 18,255 successful classified retirements, zero source-unallocated retirements, 81 refined aggregates, and success status. No asset identity, local path, hash, firmware/media byte, PC, raw trace, or provenance is retained here.
- The current-gates GCC build and full CTest/documentation gates are required before acceptance.

## Transfer

The next T390 work remains one finite **C0 direct-row regression batch** over all 81 refined aggregates, using the established Intel-primary row crosswalk. It must provide project-owned direct regressions and a complete before/after matrix before any C0-only physical-retirement decision. It may not replace this transfer with a new first-terminal repair, unbounded boot replay, board timing, or L3 claim.