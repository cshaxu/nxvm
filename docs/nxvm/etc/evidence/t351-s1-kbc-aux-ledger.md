# T351 S1 8042, Keyboard, And AUX State Ledger

## Scope and provenance

This ledger is the admission boundary for the selected PC/AT input path. It
uses the IBM PC/AT Technical Reference and Intel UPI-41/42 family manual as
behavior sources, and project-owned smokes as proof. The project does not
import controller, keyboard, mouse, Bochs, PCjs, firmware, or host-input
source. The later PS/2 controller reference only identifies the provenance of
the already selected AUX-compatible extension; it is not evidence that the
original IBM PC/AT supplied an AUX port.

Primary references are IBM *Personal Computer AT Technical Reference* (March
1984), Intel *UPI-41/42 User's Manual*, and, for the selected compatible AUX
extension only, IBM *PS/2 Model 50/60 Technical Reference*.

## One guest-visible state graph

`kbc.c` is the sole owner of the guest-visible output FIFO. `60h` dequeues one
byte, then refreshes the source for the successor head. The head's origin is
controller, keyboard, or AUX: keyboard may assert IRQ1; AUX may assert IRQ12;
controller replies assert neither. Thus a producer never publishes a private
pending byte and neither IRQ route survives dequeue, reset, or finalize without
an eligible FIFO head.

`machine.c` owns deterministic readiness. At each boundary it advances KBC
after RTC and before VADP, records `KBC_ADVANCE`, and invokes
`core_machine_kbc_advance`. It initializes ports, PIC bindings, A20/reset
callbacks, profile timing, reset, and finalize. `session.c` copies queued host
requests to the machine submit APIs; host callbacks cannot mutate a guest FIFO
during an instruction. This is the retained L3 boundary.

## Manual, source, proof, and receiver ledger

| Area | Current owner and source shape | Retained proof | T351 receiver/disposition |
| --- | --- | --- | --- |
| Ports and status | `kbc.c`: reads `60h`/`64h`, writes `60h`/`64h`; OBF, IBF, SYS, CD, KE, AUX | KBC controller and AUX-port smokes | S2 reconciles selected bits and command-input lifetime. Parity/timeout semantics need a primary-manual contract or transfer. |
| FIFO and replies | 64-byte FIFO, byte origin, four-byte KBC-owned delayed reply | controller smoke capacity and response ordering | S2 owns FIFO/full, delayed controller response, dequeue and source lifecycle; S3/S4 own device reply contents. |
| Command byte | `20h`, `60h`; IRQ1/IRQ12, keyboard/AUX disable, translation/system | controller smoke | S2: validation, interface gates, source re-evaluation, reset values. |
| Controller commands | `AA`, `AB`, `AD`, `AE`, `A7`, `A8`, `A9`, `D0`, `D1`, `D4` | controller and AUX-port smokes | S2: selected-command matrix, self/interface-test reset effects, unsupported boundary, output-port ordering. Un-sourced commands transfer. |
| Output port | `D0` reads and `D1` writes bit 1 A20 and active-low bit 0 reset | controller/A20/reset tests | S2: exactly-once callback, reset cancellation, no stale FIFO/IRQ. |
| Keyboard protocol | `FF`, `ED`, `EE`, `F0`, `F3`, `F4`, `F5`, `F6`, `F2`, `FE`; scan set 1, typematic | controller, mapper, DOS keyboard, ingress smokes | S3: ACK/RESEND/BAT/identify/LED/typematic/scan-set, queue/repeat ordering, delay, IRQ1, reset. |
| Host boundary | session queue then machine submit API | ingress and transport static gates | S3 preserves copied-event and profile mapping proof; host passthrough excluded. |
| AUX route | `D4` selects AUX; AUX FIFO origin drives status AUX and eligible IRQ12 | AUX-port and guest-AUX smokes | S2 owns controller route; S4 owns device replies. |
| Selected AUX | `FF`, `F6`, `F5`, `F4`, `F2`, `F3`, `E8`, `E9`; three-byte relative report | AUX-port, VM guest, DOS mouse smokes | S4: command/status/report/delay/reset/IRQ12 matrix. |
| Advanced AUX | no wheel handshake, scaling effect, remote/read-data, resend/error timing, capture | none | Existing `TODO(Medium) Advanced 8042 AUX protocol`; admission needs selected corpus plus manual protocol and observable report contract. |
| Profile and composition | profile config drives ports, IRQs, typematic, response timing; machine binds PIC/RAM/CPU | VM profile/composition and focused smokes | S5 proves values reach the sole owner and no alternate binding exists. |
| Reset and finalize | reset clears state/applies output port; finalize deasserts IRQ1/IRQ12 | lifecycle tests and T346 evidence | S2--S4 repair local state; S5 audits cancellation/deassert/reinit. |

## Required S2--S5 sequence

1. **S2 controller mechanics:** selected IBM 8042 commands/status, one FIFO,
   command byte, output port A20/reset, controller tests, IRQ lifecycle, and
   reset/finalize. It decides from primary material whether self test resets
   selected interface/command-byte state. It must not invent a processing delay
   merely because current IBF lifetime is synchronous.
2. **S3 keyboard device:** selected AT-compatible keyboard command and
   scan-code contract, parameter states, timing, IRQ1 and reset; retain the
   copied host boundary and one FIFO.
3. **S4 AUX device:** selected three-byte AUX path used by project consumers.
   Every command needs source/corpus proof or transfer to the advanced-AUX TODO;
   PS/2 extension is never called original PC/AT hardware.
4. **S5 composition audit:** combined ordering, full/disabled FIFO, equal-tick
   readiness, profile wiring, reset/finalize, source deassertion, and all
   transfers before T351 closure.

## Sweep and exclusions

The sweep covered tracked `kbc.*`, `machine.c`, VM session/profile/composition,
`CMakeLists.txt`, focused KBC/AUX/keyboard/mouse smokes, host ingress/transport
gates, T346 timeline evidence, T350 signal history, Queue, and TODO. No second
guest FIFO, IRQ1/IRQ12 source, or host-to-guest write path was found. Port
61/PPI/speaker, parity and I/O-channel NMI sources, generic host capture,
firmware behavior, and x87 remain outside this ledger at their named receivers.
