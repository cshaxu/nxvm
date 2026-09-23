# T399 S2 B3 DeskPro Port-61 NMI Contract

## Result

B3 is accepted as a reference-derived logical contract, not physical timing.
Read-only PCjs DeskPro-specific model facts identify port `61h` bit `04h` as
RAM/fail-safe NMI disable and bit `08h` as IOCHK NMI disable; its error-status
bits clear through a high pulse of the corresponding disable bit. The retained
D3PE/D4 evidence establishes the selected signal topology and existing Core
D4 owner. NXVM had the two enable polarities and clear edge reversed.

The existing Core D4 owner now treats both bits as active-low enables, clears
the matching latch on the high disable pulse, and resets the Model-40 logical
port state with both sources disabled. VM Model-40 remains only the selector;
no public interface, source import, firmware/media dependency, clock scalar or
physical NMI propagation claim was added.

## Project-Owned Replay

`core-machine-d4-platform-s4-smoke` covers disabled-at-reset state, explicit
active-low IOCHK enable, masked/unmasked NMI delivery, high-pulse latch clear,
active-low fail-safe delivery from the auxiliary PIT route, shutdown reset and
reset restoration. It emits `M5:T399:S2:B3-ACTIVE-LOW-NMI:OK`. The private
Model-40 composition and integration smokes prove the selected composition
continues to bind the corrected Core owner.

Focused build and CTest on 2026-08-17: all three passed: Core D4 platform,
Model-40 private composition, and Model-40 integration. The result does not
select pulse duration, propagation phase, firmware timing or physical L3.