# T351 S3 AT Keyboard Device State Machine

## Selected contract

The selected keyboard is an AT-compatible device behind the one 8042-owned
FIFO. Its command reply, scan byte, and typematic byte are keyboard-origin
only; controller and AUX origins must never influence keyboard device history.
The external serial ACK acceptance and physical line timing are not modeled:
the project-owned deterministic response delay controls when a complete
keyboard-owned reply becomes guest-visible, without creating a second buffer or
claiming electrical timing.

The IBM PC/AT Technical Reference specifies that `FEh` resends the previous
keyboard output (or the byte before a prior Resend), that `FDh` through `F7h`
are acknowledged no-operations, and that `F5h` is Set Default plus scanning
disable while `F6h` is Set Default preserving the selected scanning state.

Primary sources:

- [IBM PC/AT Technical Reference, March 1984](https://bitsavers.trailing-edge.com/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf), keyboard commands `F2h`--`FFh`, defaults, enable/disable, and RESEND.
- [IBM PC/AT Technical Reference, September 1985](https://bitsavers.informatik.uni-stuttgart.de/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf), keyboard command response and protocol detail.

## Repairs and publication boundary

`last_output_byte` was one global KBC field. It let controller-origin or
AUX-origin bytes replace the byte used by keyboard `FEh` RESEND. The repair
replaces it with keyboard-only latest and prior-byte state updated only at the
single FIFO enqueue boundary when origin is keyboard. Thus the implementation
retains one FIFO while preserving device-specific response history. Repeated
RESEND follows the documented prior-byte rule when the latest keyboard output
was `FEh`.

The keyboard default helper now owns only default configuration and typematic
cancellation; callers choose scanning state. `F5h` performs defaults then
disables scanning; `F6h` restores defaults without changing the pre-existing
scanning state; `F4h` enables scanning and clears an active typematic key;
`FFh` establishes defaults with scanning enabled after its selected reset
response. `FDh`--`F7h` are acknowledged no-operations. Invalid command and
unselected scan-set paths retain their explicit `FEh` response.

## Command and proof matrix

| Command/state | Result and owner | Proof |
| --- | --- | --- |
| `FFh` | keyboard ACK/BAT, default scan set/LED/typematic, selected scanning recovery | retained controller smoke. |
| `FEh` | keyboard-only latest/prior history; controller `D0h` reply does not affect replay | strengthened controller smoke queues a scan byte, consumes controller output, then receives the scan byte from RESEND. |
| `FDh`--`F7h` | ACK with LED/typematic/scanning unchanged | strengthened controller smoke proves `FDh`; identical switch branch owns the whole selected NOP range. |
| `EDh`, `F0h`, `F3h` | ACK then one pending parameter state; selected LED/scan-set/typematic validation | retained controller smoke. |
| `EEh`, `F2h` | echo or identify response is keyboard-origin and follows the one FIFO/delay owner | retained controller smoke. |
| `F4h`, `F5h`, `F6h` | enable, defaults+disable, or defaults with retained scanning state; typematic cancellation | strengthened controller smoke proves `F5h` state reset and `F6h`/NOP retained state. |
| scan/typematic | submit API accepts only enabled scanning, queues keyboard origin, and uses the KBC readiness clock | retained controller, mapper, ingress, and DOS keyboard smokes. |
| delay/full FIFO | delayed keyboard replies remain private until the one KBC FIFO has capacity | retained controller smoke. |
| IRQ1/reset/finalize | FIFO-head keyboard origin is the only eligible IRQ1 source; controller S2 owns interface gate and final source release | T351 S2 proof plus retained timeline and controller lifecycle coverage. |

## Similar-issue sweep and transfers

The sweep covered all keyboard command branches, pending-write states,
`core_machine_kbc_enqueue`, delayed-response publication, scan/typematic
writers, keyboard submit APIs, IRQ1 refresh, reset/finalize, host/session
mapping, and keyboard/AUX focused tests. The global last-output field was used
only by keyboard RESEND and is replaced at its owning enqueue boundary. No
controller or AUX branch consumes keyboard history.

The project retains only scan set 1. Physical serial ACK/clock timing,
unselected scan-set/device forms, error-byte generation, and host capture do
not acquire guessed semantics here. Standard AUX device protocol and IRQ12
remain S4; advanced AUX features remain the named TODO receiver.

## Verification

- `core-machine-kbc-controller-smoke` passes the expanded keyboard default,
  NOP, keyboard-only RESEND, interface, FIFO, and IRQ lifecycle matrix.
- Retained keyboard mapper, host-ingress, DOS keyboard, AUX guest, mouse, and
  T346 timeline markers remain required.
- Full current-gate and documentation governance are required before S3
  acceptance.
