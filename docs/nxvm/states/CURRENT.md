# Project Status

## Current Work

M5 T539 remains open. S3 is closed; S4 is active for RTC/CMOS extraction and
NXVM reconnection under the owner's automatic-S admission authorization.

| Task | Progress |
| --- | --- |
| T539 S4 | Active: RTC extracted/reconnected; dual-width units and default integration pass; receiving artifacts and final review pending. |

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T539 S4 after S3 P3 803c9d019. |
| Admission And Approval | Owner instructed automatic admission of each S without manual approval on 2026-09-28, after reviewing the RTC Shared/NXVM extraction and invalid-month risk. Coordinator admits this bounded batch under that authorization. |
| Objective | Extract the sole MC146818-compatible mechanism into a Types-only opaque x86 component; reconnect NXVM registers/IRQ/time/seed consumers, remove old code and contain invalid-month array indexing without claiming undocumented hardware behavior. |
| Non-goals | Other chip migrations in this S, new RTC precision claims, host clock/persistence, Lib/Common/MyNES edits, INI/media/font changes, sibling writes and new frameworks. |
| Reference Baseline | 803c9d019; S2 contracts and the rtc.c/rtc.h row in the original 81-file ledger; current eight 0539 receivers. |
| Candidate Proposal | [Independent chips](../proposals/m5-shared-chip-extraction.md), [contracts](../etc/architecture/t539-boundary-contracts.md), [ledger](../etc/evidence/t539-chip-migration-ledger.md). |
| Files And ABI Surface | Shared src/x86 and test/x86 RTC source/tests/build/guards/manifests; NXVM RTC board wiring, callers, tests, build/docs and eight 0539 EXEs. Opaque register access, copied timing inputs, IRQ callback, time advance/deadline and SQW observation. Separate Shared/NXVM commits. |
| Applicable Rules | Task Reading Set, source policy, Architecture/Coding/Document/Execution and architecture-governance then coding-governance skills; one state owner, no raw peer state, no board policy in chip, complete failure cleanup, Types vocabulary and unchanged valid calendar semantics. |
| Verification | Baseline bounds reproducer; chip-only x64/x86 suites including all input-month encodings; full NXVM units both widths; six manifests, source boundaries and applicable static/document gates; existing external integration and each profile/width boot once; eight optimized compiler-debug-stripped 0539 EXEs with PE/SHA and unchanged INIs. |
| Expected Markers | No app dependency or PIC object in Shared RTC; no NXVM private RTC fields; one register/calendar owner; board owns index/NMI/IRQ/CMOS checksum; tests retain phases/reset/alarms, with no legacy RTC route. |
| Asset Needs | Existing external profiles/media only; local RTC source evidence and manual archive; no asset import or edit. |
| Reporting Requirements | Report contract/verification progress, actual source/test delta and artifact hashes; explicitly distinguish malformed-input safety containment from hardware qualification. |
| Stop Conditions | New external licensing/asset requirement or behavior outside this batch; cannot close with a failing required consumer. Automatic admission does not waive evidence or authorize unrelated products. |
| Exit Criteria | Both RTC ledger files migrated, all consumers and tests reconnected, unsafe indexing removed at the sole calendar helper, no duplicate route; required tests/artifacts and target-separated pushes complete, followed by coordinator actual-change review. T remains open. |
| Original Owner Request | Implement chip/device architecture separation and automatically admit each subsequent S without repeated manual approval. |
| Similar-Issue Sweep | All RTC private field reads, lifecycle and IRQ bindings, selected-index/NMI separation, timing/seed/checksum consumers and calendar lookup use in actual advance plus alarm preview; classify every hit and cover both valid/invalid BCD/binary input domains. |

[Previous task history](../history/M5-T538-deployed-boot-pairs.md) retains reviewed packets
and actual-change acceptance. [Archived proposal](../history/M5-T538-deployed-boot-pairs-proposal.md),
[convergence ledger](../etc/evidence/t538-boot-pairs.md), and
[S7 evidence](../etc/evidence/t538-s7-orphan-release.md) record scope, revised
acceptance, tests and hashes.

M5 Td S174 queued the three-stage migration. Its first candidate is now T539;
the seven remaining [Queue](QUEUE.md) candidates retain their dependency order.

## Current Technical Baseline

Four fixed products remain XT, AT, Model 40 and default PC/AT; PC110 is not
runnable. Eight optimized, compiler-debug-stripped 0539 EXEs replace 0538 with
unchanged owner INIs. MyNES retains its two unchanged 0043 receivers; its T43
remains closed.

Lib/Common retain the accepted 268464d49 baseline, including the prior orphan
release repair and test-path corrections. Shared x86 now adds the S3 PIT
component at 24162ac93; NXVM source/artifacts are 797ad8887.
[S3 evidence](../etc/evidence/t539-s3-pit-extraction.md) owns the
source/artifact mapping. Full sibling parity is not claimed, and no sibling
repository was modified.

Verification: NXVM 337/337 units and 20/20 default-profile external integration
per width; all six non-default profile/width boot matrices pass once. Independent
PIT is 8/8 per width; all 66 specialized static steps and six manifests pass.
MyNES is not a PIT receiver and has no artifact input change. Cooked-history
rollback debt remains in [TODO](TODO.md). This bounded regression acceptance
does not claim complete hardware timing qualification or indefinite absence of
intermittent faults.

## Historical Context

[Earlier status](../history/M6-T41-and-prior-status-archive.md) preserves prior
deliveries and their hosting context. T538 history preserves S6's negative
qualification and S7's unclassified observations alongside the final accepted
single-pass matrix.
