# T496 S1 IBM 5160 XT Keyboard Current-Code Gap List 2

`M5:T496:S1:XT-KEYBOARD-LIST-2:OK`

| List-1 row | Current owner/path | Disposition |
| --- | --- | --- |
| K1 | `xt_ppi_keyboard` has no keyboard-device state; VM maps a host event directly to native scan bytes. | Gap: introduce one Core XT keyboard owner; do not put it in PPI or VM. |
| K2 | `xt_ppi_keyboard` currently holds a 16-byte queue. | Gap: this mirrors the documented keyboard-side buffer in the wrong owner; move queue ownership into the new device and leave PPI with only its held byte/IRQ state. |
| K3 | `core_machine_keyboard_receive_native_bytes()` submits directly to the PPI queue for XT topology. | Gap: reroute the one Core input path through the XT keyboard device, whose only downstream operation is completed-byte delivery to PPI. |
| K4 | PPI owns PB6/PB7 latches and currently gates PPI publication itself. | Partial: retain latch ownership; add a narrow Core-local line-observation/notification boundary for the keyboard device, without a second PPI owner or profile scheduler. |
| K5 | PPI already owns `60h`--`63h`, one held byte and IRQ1 assert/deassert. | Retain: this is the sole guest-visible receiver; the new device must not duplicate it. |
| K6 | The Core reports scan set 1 for XT, and the shared mapper emits set-1 bytes. | Partial: retain the one mapper entry, but the keyboard device must be the selected 101/102-key receiver of those bytes and preserve its source-qualified FIFO/sequence rules. |
| K7 | Profile selects only `CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI`; it has no independent keyboard-device construction declaration. | Gap: add one immutable device selection only after its source-backed surface is frozen. |
| K8 | No reset/BAT state exists after withdrawal of the PPI experiment. | Gap: add it only to the new keyboard device, with the documented clock-low and result sequence; no invented acknowledgement byte. |
| K9 | No XT serial-frame model or keyboard device deadline exists. | Gap: one device-local serial/deadline state must consume PPI line state and feed only completed bytes to PPI. |
| K10 | No BAT producer exists. | Gap: the keyboard device must publish the documented AA/FC result through its normal serial/byte path; its range selection is macro/L2 and its source interval remains Manual L3. |
| K11 | Session owns host-event adaptation, then uses the public Core native-byte ingress. | Retain one-way flow, but its XT receiver changes from PPI to the new Core keyboard device; VM remains unaware of PPI/IRQ/BAT state. |
| K12 | Core scheduler has no XT keyboard deadline source. | Gap: compose the one device deadline through the existing scheduler; do not create a parallel scheduler or let VM/host advance it. |

The one admitted implementation batch covers K1--K12 in the new device owner:
move the keyboard FIFO from PPI, preserve PPI's latch/IRQ receiver, consume its
line controls, and compose the documented protocol events through the existing
Core scheduler. No PPI, VM or profile-side keyboard state is added.
