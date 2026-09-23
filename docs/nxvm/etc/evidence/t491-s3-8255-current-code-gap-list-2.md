# T491 S3 8255A / IBM 5160 Current-Code Gap List 2

`M5:T491:S3:8255-LIST-2:IMPLEMENTED-PENDING-GOVERNANCE`

| List-1 rows | Current owner/path/test | Disposition |
| --- | --- | --- |
| F1--F4, F6 | `xt_ppi_keyboard.c` owns one registered `60h`--`63h` provider, mode set, BSR, Mode-0 directions and reset `99h`; XT profile supplies immutable four-port configuration. | Complete |
| F5 | The selected XT owner deliberately yields no Mode-1/2 board behavior; `xt_ppi_keyboard.h` documents this boundary. | Explicit L0 boundary |
| F7--F8, T2--T4 | Same owner queues keyboard bytes, gates delivery with PB6/PB7 and has one IRQ1 source; `core-machine-xt-ppi-keyboard-smoke` proves hold/clear/read order. | Complete except numerical timing remains L1 |
| F9, F11, F13--F14, T3 | Same owner receives typed fault input, presents PC6/PC7, requests Core NMI once and relies on the existing NMI-mask owner; focused smoke covers parity/I/O-check/mask/reset. | Complete |
| F10 | PB0/PB1 are not consumed by `xt_ppi_keyboard.c`; existing `machine_board.c` owns Core PIT2 gate/speaker state for other board paths. No XT PPI connection reaches that sole consumer. | **Finite S4 gap** |
| F15 | XT profile selects `CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI`; the 8042 route is excluded and focused smoke proves `64h` unsupported for XT. | Complete external-input boundary |
| T1 | No selected Core physical axis consumes chip electrical numbers. | L4/out of scope |

## One implementation batch

S4 adds one typed, owner-local forwarding operation from the existing XT PPI
Port-B latch to the existing Core machine-board PIT2/speaker owner. It must
update both bits atomically through that owner, preserve reset, and extend the
existing XT PPI smoke. It may not add a second speaker state, PIT route, port
provider, scheduler, keyboard owner or generic 8255 framework.

## Sweep

The audit searched all tracked `xt_ppi_keyboard`, keyboard-topology, port and
speaker/PIT-gate production/test references. `kbc.c` is the separate 8042
topology and is not an equivalent XT path. `machine_board.c` is the sole
existing Core speaker/PIT2 owner; its selected PC/AT board ingress is the
replacement target, not a template for duplicate state.
