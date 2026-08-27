# T496 S1 IBM 5160 XT Keyboard Function And Timing List 1

`M5:T496:S1:XT-KEYBOARD-LIST-1:OK`

This finite list covers the selected XT keyboard-device boundary. PPI port
semantics remain owned by T491; this list consumes only its published board
lines and completed-byte ingress.

| ID | Obligation | Authority | Level / disposition |
| --- | --- | --- | --- |
| K1 | Keyboard scan generation, make/break production, typematic and power-on self-test belong to the keyboard device. | IBM March 1986 rendered 4-22--4-24 | Manual L3 functional ownership |
| K2 | The keyboard owns its 16-byte FIFO before serial delivery; response codes bypass that FIFO. | IBM March 1986 rendered 4-24 | Manual L3 functional ownership |
| K3 | The keyboard emits a 9-bit serial frame, start then data bit 0 through bit 7. | IBM March 1986 rendered 4-32 | Manual L3 protocol |
| K4 | PB6/PB7 board controls gate the clock/clear relation; PPI owns their latches, while the keyboard device consumes their effect. | IBM March 1986 rendered 4-32; T491 F8 | Manual L3 causal relation |
| K5 | Delivered bytes reach the PPI byte latch, then the existing IRQ1 publication route; a PPI clear releases the held byte. | IBM April 1983 rendered 1-5, 1-67; T491 F7/F8/F12 | Manual L3 causal order |
| K6 | The selected 101/102-key scan mapping, including multi-byte sequences, is defined by the manual. | IBM March 1986 rendered 4-28--4-31 | Manual L3 functional relation |
| K7 | A reset is host clock-low for at least 12.5 ms, then causes reset, BAT and the appropriate completion code. | IBM March 1986 rendered 4-26 | Manual L3 timing/functional relation |
| K8 | BAT completion is AA, failure is FC, and reset-to-completion is 300--500 ms after acknowledgement. | IBM March 1986 rendered 4-25--4-26 | Manual L3 interval/result relation |
| K9 | Clock/data are open collector; host clock-low inhibits keyboard delivery. | IBM March 1986 rendered 4-32 | Manual L3 protocol relation |
| K10 | RTS requires CTS within 250 us; first edge is 60--120 us later; active clock phase is 25--50 us; stated data-valid window surrounds each edge. | IBM March 1986 rendered 4-33 | Manual L3 interval relations |
| K11 | A host event becomes a selected-keyboard scan only through the Core keyboard device; VM may not create guest-visible PPI/BAT state. | K1--K10 architecture receiver | L1 boundary pending device implementation |
| K12 | Core may compose the source-backed reset/BAT and serial event deadlines; selecting one point in a manual range is explicitly macro/L2 integration, never physical time. | K7--K10; Core time contract | Manual L3 input plus macro/L2 selection |

Rows K1--K12 are complete for the available source. External emulator behavior
is corroboration only and does not change a level or disposition. The manual
does not require an additional reset-acknowledgement byte.
