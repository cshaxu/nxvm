# T539 S8: AT Controller And Attached-Device Boundary

Baseline 23b732201. This design consumes the finite ledger's original
`kbc.c/h` pair, including its attached keyboard/AUX mechanisms and callers.
It is not a claim of a complete 8042 MCU or a new hardware timing grade.

## Owners

- Shared `devices/kbc8042`: qualified AT controller command/register behavior,
  command byte, input/output pins, keyboard translation, guest output origin,
  transport buffering and configured reply/serial delivery timing. No CPU,
  RAM, PIC, fixed PC port, board name or host input object.
- Shared `devices/keyboard`: existing AT keyboard command/parameter, scan-set,
  LED, BAT/startup and typematic state. Its commands produce bounded copied
  replies; native input and repeats use one byte transport. AUX command,
  parameter and packet state have their own `devices/ps2mouse` endpoint owner,
  not board mirrors. Its current protocol remains the implemented three-byte
  subset; extraction does not add wheel/scaling/remote-mode support.
- NXVM KBC attachment: constructs endpoints/controller, connects their public
  byte interfaces, maps 60h/64h, routes IRQ1/12, applies output-port A20/reset
  and reset pulses, and submits board-clock service units. Device-private
  objects never cross the boundary. XT protocol remains untouched.

The implementation may split endpoint files by real responsibility, not add
a general device framework or a vtable shared by unrelated chips. Controller
transport state must not be copied into endpoint or board state. Copied reply
values transfer ownership once; they are not a second independently scheduled
response path. App input chooses the keyboard's actual scan set through a
bounded observation, not by reading its internal storage.

## Order And Lifecycle To Preserve

The baseline is more than port values. Native scan admission is serialized;
ACK must precede BAT; explicit keyboard reset consumes the startup condition
so a later line release does not produce another BAT. Controller self-test
clears its existing output path. IRQ transitions belong to the byte's origin;
reading data acknowledges that origin before promoting its successor.

Existing controller response delay/status-poll ordering and keyboard serial
cadence remain explicitly configured service-unit behavior, not physical-clock
claims. Typematic retains its existing parameter formula and native break
handling. Deadline queries observe pending work, never advance it. Reset keeps
timing/bindings and releases asserted outputs; destruction occurs only after
execution stops, while signal recipients remain alive. New allocations must
unwind routes and device instances on failure.

The original pending-write enum mixes controller and keyboard transactions.
Audit interleaved controller/endpoint parameter writes before separating it;
do not silently change command routing under the label of relocation. Likewise
audit reset/flush behavior and scan/reply coincidence before splitting the
current advance routine. Any necessary behavior correction needs evidence and
an explicit packet decision, not an undocumented compatibility branch.

## Source-Verified Separation Constraints

- Pending LED/scan-set/typematic parameters survive controller commands which
  do not consume a following data byte. Controller 60h/D1h/D4h overwrite that
  pending transaction. The 96-row characterization covers all three keyboard
  parameter commands, sixteen controller interventions and both AUX-present
  configurations. Splitting enums must preserve this qualified-model routing,
  or explicitly review a correction; it is not a general hardware claim.
- Keyboard FFh currently schedules ACK before changing BAT/default state;
  F5h/F6h apply defaults before scheduling ACK. Zero-delay scheduling itself
  services pending serial input. A copied-result conversion must account for
  this order rather than silently moving all effects ahead of delivery.
- The resend history currently observes bytes accepted by the controller
  output path, after translation. The scan-set value also conditions current
  translation. These are existing model couplings to reconcile explicitly;
  neither justifies exposing all endpoint state or private peer pointers.
- AUX commands already have an independently pending parameter transaction.
  Their reply is a bounded copied value; report admission accepts a whole
  three-byte packet or none, and only successful admission changes button state.
  The old scaling flag was always reset false, never set by any command or
  caller. Removing that unreachable state retains E9h's zero scaling bit and
  the existing unsupported-command response; it does not implement scaling.

The controller borrows a dedicated connection contract: endpoint inputs are
sampled values, commands and accepted bytes are synchronous deliveries, and
IRQ/output-port/reset outputs go to the composer. This is not a shared device
vtable. The composer supplies keyboard service in the preserved transport
phase, including its scoped repeat sink; no endpoint pointer or state enters
the controller. Callbacks may deliver replies/reset stream state as documented,
not recursively issue controller commands or destroy objects.

The only direct keyboard-code dependency is the stateless Set-2/Set-1 codec
already shared by repeat classification and translation. The corpus verifier
rejects all other keyboard symbols in the controller. This avoids a duplicate
conversion table without giving the controller access to keyboard ownership.

## Proof And Removal

Pure controller/endpoint cases become standalone `test/x86/devices` tests.
Board wiring, profile clock, reset/A20 and integration cases remain NXVM and
use ports, copied external signals, delivered bytes and timing observations.
No whole-state getter is added to preserve private assertions or boot dumps.
Retain all original scenarios, including FIFO pressure, delayed responses,
translation, typematic release, AUX gating and BAT/IRQ ordering.

Audit actual bodies and all callers, not only include paths. The final evidence
maps each old test responsibility, every retained board operation, changed
ordering and allocation failure to proof, then verifies all eight receivers.
Until that evidence and actual-change review exist, this ledger batch remains
in progress and the accepted S7 baseline remains the last delivered source.
