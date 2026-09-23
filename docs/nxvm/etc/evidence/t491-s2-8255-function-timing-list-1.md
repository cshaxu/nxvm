# T491 S2 8255A / IBM 5160 Function And Timing List 1

`M5:T491:S2:8255-LIST-1:IMPLEMENTED-PENDING-GOVERNANCE`

Intel 8255A-5 document 231308-004 page 1 is visually verified; its remaining
pages are visual-only source material. IBM 5160 Technical Reference section
1, printed pages 1-8 through 1-10, is the board authority.

| ID | Function/timing obligation | Authority | Level |
| --- | --- | --- | --- |
| F1 | Decode four consecutive PPI ports `60h`--`63h`: A, B, C, control. | IBM I/O map | Manual L3 |
| F2 | Preserve 8255A data-bus reads/writes and A1:A0 port/control selection. | Intel p.1 block diagram | Manual L3 |
| F3 | Support mode-set control words and Port-C BSR semantics. | Intel p.1 feature/block diagram | Manual L3 |
| F4 | Mode 0 direction follows the control word for ports A/B and C halves. | Intel p.1 | Manual L3 |
| F5 | Modes 1/2 handshaking are chip-defined but have no selected IBM XT board wiring. | Intel p.1; IBM map | L0 selected-board behavior |
| F6 | Reset/control power-on mode is `99h` for the selected IBM board. | IBM p.1-10 | Manual L3 |
| F7 | Port A reads the delivered keyboard scan byte under selected Mode 0 input wiring. | IBM p.1-10 | Manual L3 |
| F8 | Port B bit 6 releases/holds the keyboard clock; bit 7 enables/clears keyboard delivery. | IBM p.1-10 | Manual L3 |
| F9 | Port B bits 4/5 enable RAM parity and I/O channel-check reporting. | IBM p.1-10 | Manual L3 |
| F10 | Port B bit 0 controls PIT channel-2 gate; bit 1 drives speaker data. | IBM p.1-10; PIT board mapping | Manual L3 |
| F11 | Port C bits 6/7 report I/O channel check and RAM parity respectively. | IBM p.1-10 | Manual L3 |
| F12 | A complete received keyboard scan code asserts IRQ1; port-A read/keyboard-clear releases it. | IBM p.1-4, p.1-10 | Manual L3 logic order |
| F13 | Asserted parity/I/O-check with its Port-B enable produces one NMI request. | IBM p.1-10 and NMI description | Manual L3 logic order |
| F14 | NMI mask at `A0h` is a separate board register; it gates CPU delivery, not PPI state. | IBM p.1-8 note | Manual L3 |
| F15 | Keyboard serial transport, scan generation and 8048 internals are keyboard-device behavior, not 8255/8042 controller behavior. | IBM p.1-65 | External-input L3 |
| T1 | Chip RD/WR and setup/hold electrical numbers are source-backed but no Core physical axis consumes them. | Intel 8255A-5 timing pages | L4/out of scope |
| T2 | Keyboard byte completion precedes IRQ1, PPI port-A observation and clear/release. | IBM system description plus F7/F8 | Manual L3 logic order |
| T3 | Fault assertion precedes Port-C status and NMI request; deassert/re-enable allows a later edge. | IBM F9/F11/NMI wiring | Manual L3 logic order |
| T4 | Keyboard clock/clear gating has no IBM numerical interval; only its causal order is claimed. | IBM p.1-10 | L1 |

Rows F1--F15 and T1--T4 are the complete selected 8255/XT PPI unit surface.
PIT speaker/gate consumption remains owned by its existing PIT route; S3 maps
the interaction but may not duplicate that state.
