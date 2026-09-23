# T401 S76 Independent CPU-Audit Closure Review

## Decision

T401 does **not** close in S76. The source graph and accepted S1--S75 evidence
support a finite decoder-map, semantic-family and source/nonphysical timing
classification result, but they do not yet satisfy the proposal's stricter
completion predicate: one four-profile requirement-to-source matrix organized
by concrete decoded form, with a primary-manual citation and a complete
per-form conclusion/disposition. Passing family smokes and a class-level map
cannot substitute for that matrix.

## Requirement-to-evidence review

| Original T401 requirement | Current evidence | S76 result |
| --- | --- | --- |
| Freeze the selected CPU universe and classify decoder availability. | S1 ledger universe; S3 256-slot `0F` matrix; S4 primary matrix; S72/S73 finite-map reconciliations; S5 lexical rules. | Satisfied for the selected primary, `0F`, FPU/WAIT and prefix spaces. |
| Audit decode, prefixes, ModR/M, immediate consumption, profile gates and reserved forms. | S5, S7--S10, S49, S57--S58 and S66--S71; current lexical/UD smokes. | Satisfied as family evidence, including the repaired TR6 legal-neighbour regression. |
| Audit state, FLAGS, transaction, fault, restart, mode and selected system forms. | S6--S69 family evidence and retained current mode/fault/descriptor/I/O smokes. | Family-level evidence is accepted; no reproduced shared Core semantic defect remains in this review. |
| Give every successful form/context a source-backed row, formula/range model, or owner-local nonphysical observation. | T357/T359/T360 selected source ledgers and transfers; S75 classifier/observation matrix; four-profile `SOURCE_UNALLOCATED` physical-rejection smoke. | The owner-local nonphysical boundary is now explicit, but its evidence is class-level rather than the proposal-required concrete-form matrix. |
| Reproduce each discovered defect, add focused proof, perform profile/similar-form sweep, and run current gate. | S2/S4/S7--S9/S58/S66--S70 repairs, S75 two test repairs, and current-gate records. | Satisfied for the observed defect classes. |
| Transfer x87 breadth, unselected extensions and physical board timing without overclaim. | S6/T316 S65 FPU provider boundary; S72 reserved/later map; S75 and DeskPro closure evidence. | Satisfied; no numerical x87, board-cycle or DeskPro-L3 result is claimed. |
| Produce the complete four-profile per-decoded-form requirement-to-source matrix with Intel citations and final conclusion. | T401 ledger is organized principally by audit family and references retained manuals, but many accepted families do not contain a row for every decoded/profile/mode form with all required schema fields. | **Open.** This is the one remaining T401 closure condition. |

## Required receiver

**T401 S77: concrete-form Intel requirement matrix** is the earliest receiver.
It must consume the frozen S1 universe and make the existing family evidence
addressable at the proposal's required row granularity: CPU/profile/form,
manual edition/section/table, current owner, timing disposition, focused proof,
conclusion and repair/transfer. It may consolidate identical rows only where
it explicitly records the full finite opcode/form/profile membership. A gap
found there receives a bounded corrective S or an explicit receiver; no valid
form may be hidden behind a metadata gate or generic successful fallback.

S77 is documentation/evidence work unless the matrix finds a source defect.
It neither changes Core/VM interfaces nor admits a physical timing model. The
separate DeskPro board/device/firmware receivers remain outside T401 and retain
their original/reference/generic confidence labels.

## Verification

S76 reviewed the current `core_machine_instruction_cost()` and copied
retirement-observation publication boundary, the original T401 proposal,
S1--S75 ledger, latest source/test graph, current Queue/TODO state and the
accepted full current gate after S75 (285/285). Documentation governance and
`git diff --check` pass for this review. The conclusion intentionally remains
open until S77 makes the required per-form matrix durable.