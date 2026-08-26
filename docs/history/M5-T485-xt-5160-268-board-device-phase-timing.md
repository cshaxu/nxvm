# M5 T485 IBM PC/XT 5160-268 Board And Device Phase Timing

T485 consumes T484's closed selected-device functional routes.  It may only
connect primary-source board/device timing inputs to the one Core time axis;
it must not create an XT scheduler, AT alias, guessed timing value or host-time
writer.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: every selected timing path has one source-labelled fact, current Core receiver and explicit L2/blocking disposition; only PIT has an immediately eligible rational-clock receiver. [Evidence](../etc/evidence/t485-s1-xt-board-phase-source-ledger.md). |
| S2 | Accepted: IBM's `1/4` PIT relation requires the 4.77 MHz XT board-clock axis; current instruction ticks cannot receive it. [Evidence](../etc/evidence/t485-s2-xt-pit-common-axis-boundary.md). |
| S3 | Accepted: Intel/IBM facts map to the single Core time publisher; generic compatibility timing is explicitly nonphysical, and Table-2-21/external-cycle qualification transfers to S4/S5. [Evidence](../etc/evidence/t485-s3-8088-common-axis-ledger.md). |
| S4 | Accepted: the exhaustive current selector ledger proves no 8088 Table-2-21 form is selected today and identifies its one private Core transfer-count receiver. [Evidence](../etc/evidence/t485-s4-8088-table-2-21-selector-ledger.md). |
| S5 | Accepted: current decoded state has no complete word-transfer plan, so an evaluator cannot lawfully reuse local helpers or external byte traces. [Evidence](../etc/evidence/t485-s5-8088-transfer-plan-receiver.md). |
| S6 | Accepted: the plan is a one-use private timing result, never machine state; complete/incomplete source inputs are frozen for immediate consumption. [Evidence](../etc/evidence/t485-s6-private-source-transfer-plan.md). |
| S7 | Accepted: one private 8088 primary-form transfer plan feeds the sole source evaluator before compatibility; three rendered Table-2-21 forms have exact 3/19/30-clock proof, while incomplete forms remain unallocated. The complete 299-test current gate passes. [Evidence](../etc/evidence/t485-s7-8088-primary-source-evaluator.md). |
| S8 | Accepted: rendered Table-2-21 and the complete decoded string sweep prove exact 8088 word-transfer plans for all ten MOVS/CMPS/STOS/LODS/SCAS byte/word forms and their repeat phases. The one evaluator receiver is ready; port I/O and physical XT qualification remain excluded. [Evidence](../etc/evidence/t485-s8-8088-string-transfer-ledger.md). |
| S9 | Accepted: one private plan and the retained string evaluator source-classify every S8 byte/word and repeat form; 299 current-gate tests pass. Control/stack, dynamic, fault/interrupt and board-cycle batches remain explicitly separate. [Evidence](../etc/evidence/t485-s9-8088-string-source-evaluator.md). |
