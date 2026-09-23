# T396 S2: DeskPro Board-Timing Task-Closure Audit

`M5:T396:S2:TASK-CLOSURE-AUDIT:OK`

## Proposal Reconciliation

| Board-timing proposal requirement | Direct current evidence | Closure disposition |
| --- | --- | --- |
| 16 MHz CPU-related time and 1.19318 MHz dual-8254 TIMCLK, rational conversion and reset phase | D3PE facts and T396 S1 ledger; both Model-40 constructors remain deterministic 1:1; T394 C0 qualification prevents unallocated physical publication but selects no Model-40 descriptor | The sourced frequency facts are retained, but no Core-tick conversion or phase exists. Transfer the physical-clock consumer contract to the next DeskPro physical-device/firmware timing candidate. |
| DCLK/BCLK availability, dual-8237A arbitration and DMA wait placement | S1 ledger identifies 4 MHz DCLK and one wait, Core DMA owner and logical HOLD/ack/release | Core logical arbitration is retained. DCLK-to-transaction conversion and wait placement transfer to the next DeskPro physical-device/firmware timing candidate. |
| Dual PIT, PIC, RTC/CMOS, 8042, port 61h, reset and NMI visibility/order | S1 ledger names all six rational domains, three callback phases, reset order and D4/port-61 functional owner | Deterministic callback order/reset is retained. Electrical phase, propagation, settle, refresh/failsafe and source-backed device availability transfer to the next DeskPro physical-device/firmware timing candidate. |
| Primary source before secondary observation | D3PE fact ledger; T387 S3/S4 bridge contract and explicit rejection of local generic/insufficient 86Box, PCjs and MAME references | Complete non-substitution decision. A later receiver may admit a primary-constrained observable probe; generic AT/emulator output cannot authorize a Model-40 scalar. |
| Core/VM ownership and no local scheduler | S1 publisher/consumer ledger; Core clock/timeline/transaction ownership and VM's two deterministic construction declarations | Complete. No Core/VM interface or runnable behavior changed. |
| Required regressions and current-gate verification | Five focused timing tests passed; `cmake --build --preset current-gates-gcc` completed all 68 declared checks; documentation governance passed | Complete on the unchanged runnable graph. |

## Similar-Issue And Boundary Sweep

The closure reviewed both Model-40 constructors, all six Core clock consumers,
all three timeline callbacks, Core reset/publish paths, D3PE fact records,
T387/T389/T394 transfers, the physical-device/firmware proposal and L3 audit
proposal. Model 339 and generic PC/AT plans remain separate profile contracts.
No source or production defect was found; no new code repair is required.

## Closure Decision

T396 closes as a truthful non-publication result. It establishes that a physical
board scalar cannot be inferred from an accepted CPU qualification key, a
nominal 80386/16 label, DCLK/TIMCLK facts or generic emulator behavior. The
neutral deterministic Model-40 plan is retained. The next queue candidate,
[DeskPro 386 physical-device and firmware-timing closure](../../history/M5-T397-deskpro-386-physical-device-firmware-timing-closure-proposal.md), receives every unresolved physical availability, wait, phase and device/firmware visibility fact. The later Model-L3 audit remains blocked until that candidate closes; this task makes no L3 claim.