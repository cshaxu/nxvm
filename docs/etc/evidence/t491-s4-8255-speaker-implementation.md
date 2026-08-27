# T491 S4 IBM 5160 8255 Speaker/PIT2 Implementation

`M5:T491:S4:XT-PPI-SPEAKER:IMPLEMENTED-PENDING-GATE`

The existing XT PPI owner remains the sole Port-B latch owner. It now publishes
only PB0 (`timer_gate`) and PB1 (`data_enabled`) through one private callback.
The existing Core board owner stores those two consumed board lines, drives the
existing shared PIT channel-2 gate, and computes the existing copied speaker
observation. No full Port-B copy, new port provider, PPI/PIT state, scheduler
or keyboard/NMI path was introduced.

The shared helpers are named for the speaker responsibility, not PC/AT Port B:
XT, planar-parity, and DeskPro sources select the same existing consumer path.
This is a rename only; it removes the now-misleading platform-specific name
without adding an abstraction layer.

`core-machine-xt-ppi-keyboard-smoke` now proves disabled reset lines, PB1 data
enable, PB0 gate enable and reset restoration; focused PPI/PIT CTest passes
3/3. A fresh, separately logged full current gate passes 300/300. The retained
L1 keyboard-clock interval and L4 electrical timing are unchanged.
