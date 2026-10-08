# T546 S22 Cross-Family Regression And Source Convergence

## Method And Boundary

S22 is the final repair-audit receiver, not a replacement for the per-family
manual records.  It reads the eighteen receivers transferred by T544,
cross-references their S1--S21 owners, then checks the current timing routes
and the five result corpora.  A passing catalog proves only its recorded
contexts; it is not claimed as exhaustive proof of every architectural state.

## Complete Receiver Disposition

| T544 receiver | Current T546 owner | Current disposition |
| --- | --- | --- |
| Reset and architectural images | S1 | Repaired at the sole reset/image producers. |
| Runtime decode/admission | S2 | Repaired at fetch/decode admission; no caller-local decode path. |
| Effective address and segment span | S3 | Repaired at address/span preparation. |
| Admission versus next fetch | S4 | Repaired at instruction completion/following-fetch boundary. |
| Host arithmetic/count | S5 | Repaired at host-width/count producers. |
| Stack/frame publication | S6 | Repaired at stack/frame owners. |
| FLAGS privilege and return | S8 | Repaired at FLAGS load/image and return producers. |
| Asynchronous arbitration | S9 | Repaired at the one NMI/INTR/debug arbiter. |
| Exception delivery/shutdown | S10 | Repaired at finalizer/shutdown owners. |
| Descriptor/query/table | S14 | Repaired at descriptor query/table owners. |
| Gate and outer return | S15 | Repaired at control-transfer predicates and cleanup. |
| Task transition | S16 | Repaired at the staged transition owner. |
| Paging and implicit references | S17 | Repaired at the physical GDTR/IDTR and page-walk routes. |
| String and port restart | S18 | Repaired at the per-element restart owner. |
| CPU external/NPX/bus | S19 | Repaired at Core-owned external wait arbitration. |
| Scalar/formula/transfer timing | S20 | Repaired at the sole successful-retirement selector. |
| Retirement/external waits | S21 | Repaired at the one pending-retirement wait owner. |
| Regression/oracle source | S22 | Current tests and source labels reconciled below; no new implementation owner is needed. |

## Current Timing-Tier Sweep

The current result corpora contain no selected successful route marked
`source_timing_unallocated` and no failed row:

| Profile corpus | Rows | Unallocated | Failed |
| --- | ---: | ---: | ---: |
| 8086 | 1,053 | 0 | 0 |
| 8088 | 989 | 0 | 0 |
| 80186 | 616 | 0 | 0 |
| 80286 | 771 | 0 | 0 |
| 80386DX | 1,413 | 0 | 0 |

The 80386 VM86 `POP FS`/`POP GS` L1 hole named by T544 is no longer present:
S20 supplies the existing Manual-L3 seven-clock real/VM row.  The remaining
one-tick value in the timing owner is only the explicit failure/unallocated
sentinel; it is not selected by any current successful catalog route.

Existing L2 classifications remain intentional rather than hidden L1:

- source ranges and operand-dependent multiply/divide choices;
- the unavailable-next-instruction control transfer component;
- CPU/NPX service arbitration where the CPU manual does not define complete
  coprocessor service time.

None is relabelled L3 or converted into a physical board duration.  The Core
L1 compatibility escape is a separately declared turbo/no-deadline policy;
it is not CPU successful-retirement timing and does not become an instruction
timing classification through this audit.

## Source Conflicts

The retained historical conflicts are not silently averaged.  The current
owners use their sourced resolution where one exists (for example restrictive
PDE U/S, VM86 IRET and the outer-return predicates).  The remaining
edition/undefined-form disagreements are retained as non-applicable or
undefined-domain limits, not as legal successful forms with invented timing.
No unfixable legal-success L1 or necessary tier downgrade was found.

## Verification

- Five timing-manifest runners passed on x64: 8086, 8088, 80186, 80286 and
  80386DX.
- The same five runners passed on x86.
- JSON corpus scan counted the 4,842 current rows above with zero
  `source_timing_unallocated` and zero failed rows.  (The 8088 corpus is a
  distinct 989-row source manifest; it is not duplicated in the legacy
  8086 file.)
- Earlier S21 complete repository-only units remain 506/506 on each width;
  this S adds no production code.

S23 remains the only open T546 receiver: final actual-diff review, complete
dual-width qualification and the original external integration suite.  This
record does not claim that an audit catalog alone proves all unenumerated CPU
contexts.

