# T403 S2: Keyboard/8042 Functional Closure Audit

`M5:T403:S2:CLOSURE-AUDIT:OK`

## Decision

T403 closes the selected shared keyboard serial-endpoint and 8042 flow-control
functional universe. It does not close physical keyboard/8042/DeskPro timing or
Model-L3 readiness.

## Reconciliation

| Selected member | Evidence and disposition |
| --- | --- |
| Atomic mapped native sequences and capacity | T403 S1 endpoint admission owns native bytes; full-output break proof accepts the complete sequence without partial publication. Accepted. |
| Set-1/Set-2, translation, extended/pause and break parsing | Existing controller smoke covers both scan paths, translation, extended and Pause; S1 preserves their owner. Accepted. |
| Typematic start/cancel/repeat | Existing timing regression plus S1 congestion proof covers start, repeat and break-at-admission cancellation. Accepted. |
| 60h/64h, IRQ1/IRQ12 and output ordering | Existing FIFO remains sole CPU-visible owner; KBC/AUX/controller smokes cover status, origins and IRQ routes. Accepted. |
| Commands, delayed responses, scan state, reset and resend | Controller smoke covers command transitions, delayed replies, reset and resend; endpoint is reset with private KBC data. Accepted. |
| AUX coexistence | AUX port and VM guest smokes cover packets, command routing and IRQ12; unchanged owner. Accepted. |
| VM mapper and guest consumer | mapper and DOS keyboard smokes pass; VM retains mapping-only ingress. Accepted. |
| Physical serial rate, exact FIFO/microcode, wire flow control, IRQ latency and DeskPro board timing | No admissible physical evidence; explicitly transferred to the physical-observable device timing receiver. |

## Closure statement

Actual P1 review finds one Core KBC owner and no Core/VM public-interface or
profile-specific path. Focused and full 285/285 current-gate evidence pass.
All selected functional members are proven or transferred; T403 may close only
at this functional scope, with no physical or L3 assertion.
