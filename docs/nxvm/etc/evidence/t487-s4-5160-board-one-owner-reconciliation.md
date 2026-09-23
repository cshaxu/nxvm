# T487 S4 IBM 5160 Board One-Owner Reconciliation

`M5:T487:S4:IBM-5160-BOARD-ONE-OWNER:ACCEPTANCE-CANDIDATE`

No production change is correct for this batch. The visual IBM source supplies
board cycle relations, while the visual Intel source defines Table 2-21 as the
clock-period total for an instruction form and includes its transfer terms.
The existing 8088 Core timing selector is therefore the sole clock-total
owner. Adding IBM's four/five clocks through `external_access_wait_windows`
would create an incorrect second charge.

PCjs was inspected only as bounded corroboration: it labels its instruction
cycle counts approximate and identifies prefetch and wait states as obstacles
to exact timing. The inspected 86Box IBM-5160 board source supplies no
independent reusable bus-phase scheduler. Neither provides Other-L3 authority.

| Disposition | Result |
| --- | --- |
| Manual-L3 | IBM oscillator, CLK relation, stated board-cycle/refresh relations, reset polarity, map, channel and NMI facts remain exactly recorded in S2. Intel owns each selected 8088 instruction-form clock total. |
| Retained owner | Core CPU timing selector/retirement remains the only clock-total route; Core PPI/NMI, memory/ROM and topology routes remain singular. |
| Explicit L2 | Sub-instruction IBM bus phase, card READY behavior, controller service duration and full-machine physical pacing. No source-qualified implementation model exists. |
| Rejected path | XT profile additive waits, a new cycle classifier, a profile-owned clock/scheduler, and host-derived Core advancement. |

The zero-line production result is intentional simplification: it avoids a
duplicate timing path and preserves every current owner. T487 may proceed to
closure audit; later controller units consume their own source-backed ratios
without reinterpreting Intel instruction totals.
