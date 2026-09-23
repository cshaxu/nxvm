# T399 S2 B3 Acceptance Audit

The coordinator reviewed S2 P2 `d7a09b7a` against the active packet, PCjs
DeskPro-only port-61 facts, retained D3PE topology, and the actual Core/VM
diff. The correction is confined to the existing Core D4 owner: bits `04h` and
`08h` are active-low enables, their high pulses clear the corresponding
latched status, and reset disables both sources. VM composition and public ABI
are unchanged.

Focused Core D4, Model-40 private-composition and Model-40 integration CTests
all pass; documentation governance passes. B3 is accepted as
`M5:T399:S2:B3-CONTRACT-RECONCILED`. It does not claim electrical pulse width,
propagation phase, firmware timing or physical L3. Those remain the explicit
physical-observable receiver.