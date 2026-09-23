# T404 S5: Public Host Input Contract

`M5:T404:S5:HOST-INPUT-CONTRACT:OK`

The VM session supports exactly two public host event kinds: key and relative
mouse. Both are copied into the ordered request transport and consumed only at
an execution boundary. An unknown enum value was previously reported as
accepted even though the VM consumer ignored it. S5 rejects that value at the
VM boundary with `TYPE_STATUS_INVALID_ARGUMENT`; Core's generic input source
remains capable of serving other owners with their own event contracts.

The existing host-ingress smoke proves supported key ingress, platform ingress,
unknown-kind rejection and an unchanged empty queue after rejection. Existing
AUX and DOS mouse regressions retain supported relative-mouse proof. Physical
keyboard, mouse, 8042 and board timing remain transferred to their existing
DeskPro/L3 receivers.

Verification: focused host-ingress smoke pass; full current gate 286/286 pass;
documentation governance pass before acceptance.