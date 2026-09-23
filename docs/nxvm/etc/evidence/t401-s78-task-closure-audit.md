# T401 S78: Independent Task-Closure Audit

## Decision

T401 closes as a bounded four-profile CPU semantic and source-timing audit.
This conclusion is deliberately narrower than complete instruction correctness,
physical CPU cycle accuracy, numerical x87 support, clone compatibility or a
DeskPro Model-L3 result.

## Independent reconciliation

| Original completion condition | S78 evidence and decision |
| --- | --- |
| Complete selected decoded-form universe and no unclassified reachable row. | S1 freezes 256 primary bytes, 256 `0F` bytes, eight ESC families and the shared prefix path. S72/S73 reconcile every metadata-valid primary/secondary member; S77 refines every family to its complete finite opcode, extension and profile set. Accepted. |
| Concrete four-profile requirement-to-source matrix with Intel authority, owner, proof, timing and conclusion. | S77 supplies the required matrix: each finite row names its Intel 8086/80186/80286/80386 manual locus, current owner, focused proof and source/nonphysical/external disposition. Ranges and groups explicitly define full membership rather than a representative sample. Accepted. |
| Decode, prefix, ModR/M, immediate, profile gate and reserved-form coverage. | S5/S7--S10/S49/S57--S58/S66--S71 and the S77 rows cover lexical handling, grouped extensions, invalid metadata forms, `0F` availability and legal TR6/TR7 versus rejected TR0--TR5. Accepted. |
| State, FLAGS, transactions, fault/restart and selected system/control/debug/test forms. | S6--S69 family evidence and current fault/descriptor/I/O regressions are mapped by S77 to their concrete forms. No unrepaired shared-Core semantic discrepancy was found in this independent review. Accepted for the frozen selected surface. |
| One explicit timing disposition for every successful selected form/context; no silent fallback. | T357/T359/T360 retain authoritative exact/formula/range source rows. S75 makes every remaining successful selected form publish `SOURCE_UNALLOCATED`, with its source form/origin and one containment tick, and physical execution rejects before elapsed or timeline publication. The four-profile fallback smoke covers 8086, 80186, 80286 and 80386. Accepted as a nonphysical observation, never as a measured cycle. |
| Reproductions, focused regressions and defect-class sweep. | S2/S4/S7--S9/S58/S66--S70 document the discovered repairs and variants. S75 also corrected the stale legal TR regression and added the cross-profile unallocated-observation regression. Accepted for discovered defect classes. |
| Full current-gate and inventory verification. | S78 reran `verify-t359-instruction-timing-inventory`, `verify-t360-timing-source-inventory`, and the full current CTest gate; all pass, with the full gate at 285/285. Accepted. |
| Explicit external transfers. | WAIT/ESC numerical provider breadth remains at the S6/T316 S65 FPU boundary. Unsupported/later forms retain the S72/S73 rejection transfer. Physical prefetch/cache/READY, bus, board, device and DeskPro timing remain outside T401 and retain their existing DeskPro receivers. Accepted without a physical claim. |

## Current-source spot check

The review inspected the sole selected successful-retirement classifier,
`core_machine_instruction_cost()`, its copied publication in
`core_machine_retirement_observation_publish()`, and physical rejection in
`core_machine_run()`. The classifier clears its observation state for every
attempt, selects one source classifier family, and exposes fallback through
`source_timing_unallocated`; it does not manufacture a source row. The
publication boundary preserves the diagnostic record while the physical path
rejects the nonphysical form before elapsed/timeline visibility. This matches
S75 and the S77 source/nonphysical matrix.

## Retained boundaries and next receiver

T401 transfers, rather than resolves, numerical x87 execution, unsupported
extensions, physical instruction-cycle timing, prefetch/cache/READY behavior,
DMA/ISA/device timing and all DeskPro board/firmware/L3 claims. The next
DeskPro work must begin from the retained Model-40 closure context and state
whether any proposed result is original-source, reference-derived or generic;
T401 supplies only CPU semantic/source-timing hygiene to that work.

## Closure statement

Every frozen selected CPU semantic and source-timing row is now proven by
primary-manual/current-owner evidence, repaired with regression evidence,
explicitly nonphysical with a published observation, rejected as invalid or
profile-inapplicable, or transferred to a named external boundary. T401 is
therefore closed at that bounded scope.