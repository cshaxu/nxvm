# T450 S2 Controller-Scoped Execution Ledger

## Fixed Universe

T450 S1 admits exactly eight controller boundaries. T450 S2 assigns two ordered
subtasks to each boundary and preserves S19 solely for coverage reconciliation.
The first records original function/timing facts; the second audits only those
facts against current code. No controller S may add a generic device abstraction
or consume another controller's unclassified rows.

| Subtask | Controller boundary | Source baseline | Required outputs |
| --- | --- | --- | --- |
| S3 | PIC 8259A original list | Intel 8259A plus IBM AT wiring | Complete source/function/timing checklist. |
| S4 | PIC 8259A gap audit | S3 list and Core PIC owner | Row-level code/test disposition. |
| S5 | DMA 8237A original list | Intel 8237A plus IBM AT wiring | Complete source/function/timing checklist. |
| S6 | DMA 8237A gap audit | S5 list and Core DMA owner | Row-level code/test disposition. |
| S7 | PIT 8254 original list | Intel 8254 plus IBM AT wiring | Complete source/function/timing checklist. |
| S8 | PIT 8254 gap audit | S7 list and Core PIT owner | Row-level code/test disposition. |
| S9 | RTC/CMOS original list | Motorola MC146818A plus IBM AT wiring | Complete source/function/timing checklist. |
| S10 | RTC/CMOS gap audit | S9 list and Core RTC owner | Row-level code/test disposition. |
| S11 | KBC 8042/NMI original list | Intel UPI-42 plus IBM AT and keyboard facts | Complete source/function/timing checklist. |
| S12 | KBC 8042/NMI gap audit | S11 list and Core KBC owner | Row-level code/test disposition. |
| S13 | uPD765 FDC/media original list | NEC uPD765 plus IBM AT and selected media facts | Complete source/function/timing checklist. |
| S14 | uPD765 FDC/media gap audit | S13 list and Core FDC/media owners | Row-level code/test disposition. |
| S15 | VADP original list | IBM Options and Adapters Volume 2 plus selected adapter personality | Complete source/function/timing checklist. |
| S16 | VADP gap audit | S15 list and Core VADP owner | Row-level code/test disposition. |
| S17 | HDC/ATA original list | ATA-3 plus selected ATA controller/backing-media facts | Complete source/function/timing checklist; blocked rows remain explicit. |
| S18 | HDC/ATA gap audit | S17 list and Core HDC/media owners | Row-level code/test disposition. |
| S19 | Cross-controller audit | T450 S1--S18 evidence | Completeness, duplicate-row, ownership and transfer audit; no new function rows. |

## Required Checklist Columns

Every odd-numbered original-list S uses source identity and page, finite
function/command/register/mode, deterministic timing formula or explicit
absence, reset/cancellation, IRQ/DRQ or equivalent relation, and an L3/L2 or
blocked disposition. Its following gap-audit S adds current owner path and
test, current implementation disposition, and exactly one later implementation
receiver or blocked rationale.

Shared board facts are cited in each dependent controller row but have one
source identity in T450 S1. A shared IRQ, DRQ, clock or backing-media fact never
creates shared mutable state or a second production owner.
