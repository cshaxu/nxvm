# T496 S2 XT Keyboard One-Owner Implementation

`M5:T496:S2:XT-KEYBOARD-OWNER:OK`

The new Core XT keyboard owner is the only holder of keyboard FIFO, reset/BAT,
serial-progress and deadline state. The completed PPI owner now retains only
the selected board ports, the received-byte latch, IRQ1 and its independent
speaker/NMI work. Its two narrow callbacks report its already-owned PB6/PB7
line state and byte release; they do not transfer PPI ownership or add a
second scheduler.

The existing public Core native-input operation now enters the XT device before
any PPI operation. The device's sole downstream operation submits one completed
byte to the PPI. There is no XT host-to-PPI queue route.

The device also owns the manual FIFO-overrun rule: an input sequence that does
not wholly fit is discarded, with one `FF` overrun code queued instead (or
replacing the final queued byte when full). It is not a PPI error path.

IBM's 12.5 ms reset minimum, 300--500 ms BAT interval, AA/FC result relation,
9-bit frame and clock intervals remain Manual L3 facts. The selected healthy
keyboard path publishes AA. No keyboard-failure predicate is sourced by the
selected board, so S2 does not invent an FC injection or a profile setter; the
FC condition remains an explicit device-internal 8048-model boundary. The
implementation uses the documented lower bounds for deterministic scheduling
on the selected macro axis; that conversion is explicitly macro/L2, not a
wall-clock claim. It does not create the previously assumed extra
reset-acknowledgement byte.

Focused `core-machine-xt-ppi-keyboard-smoke` passes after proving the Core
deadline becomes observable after qualified reset release, BAT delivers AA
through PPI/IRQ1, and an ordinary native scan byte occupies the device FIFO,
traverses serial progress and then reaches PPI. The retained PPI and Model-268
profile regressions also pass. The same smoke test rejects a 12.499 ms reset,
then accepts the 12.5 ms minimum, and covers the full-FIFO `FF` replacement.
No BYOB or DOS boot conclusion belongs to S2.
