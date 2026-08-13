# T351 S4 Selected AUX Device State Machine

## Selected device boundary

NXVM retains a selected PS/2-compatible, standard three-byte relative AUX
device behind the PC/AT 8042 controller. This is a compatible extension for
the existing guest mouse consumers, not a claim that the original IBM PC/AT
had an AUX port. The controller owns `D4h`, the one FIFO, origin/status, and
IRQ12 eligibility; the AUX state machine owns only its selected command,
parameter, response, and report state.

The selected contract is grounded in the PS/2 controller/device material and
the project-owned port, guest, and DOS mouse probes. No reference source,
firmware, or host-capture implementation is imported.

- [IBM PS/2 Model 50/60 Technical Reference, keyboard-controller material](https://www.ardent-tool.com/docs/pdf/ps2_50-60_techref_ch4a_system_board_io_controllers_keyboard.pdf), consulted only for the compatible AUX controller/device extension.
- [IBM PC/AT Technical Reference](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf), retained solely for the underlying PC/AT 8042 boundary, not as proof of an AUX device.

## Command, report, and lifecycle matrix

| Selected route | Owner and observable result | Proof |
| --- | --- | --- |
| `D4h` | controller selects exactly one subsequent AUX-device byte; no second FIFO or source | T351 S2 controller evidence and AUX port smoke. |
| `FFh` | ACK, BAT `AAh`, device ID `00h`; reporting/buttons/resolution/rate return to defaults | strengthened AUX port smoke. |
| `F6h`, `F5h`, `F4h` | defaults, reporting disable, or reporting enable; report submit rejects while disabled | AUX port smoke. |
| `F2h`, `F3h`, `E8h`, `E9h` | identify, sample-rate parameter validation, resolution validation, and status/resolution/rate response | AUX port smoke. |
| invalid command/parameter | selected `FEh` response with no guessed state publication | AUX port smoke. |
| report | one three-byte packet: button bits, sign/overflow clipping, no-change suppression, and only enabled/reporting submit | AUX port smoke and guest/DOS mouse probes. |
| command byte / `A7h` / `A8h` | AUX-origin OBF remains readable but IRQ12 is gated by command byte and interface enable; keyboard IRQ1 is distinct | controller and AUX port smokes. |
| response delay and full FIFO | replies remain KBC-owned until deterministic advance/capacity; a three-byte report does not partially publish or update buttons when less than three FIFO slots remain | strengthened AUX port smoke. |
| reset/finalize | reset applies AUX defaults; finalize deasserts the KBC-owned IRQ12 source without redefining already-latched PIC state | AUX port smoke and T346 timeline evidence. |
| copied guest ingress | host relative event crosses session request transport at an execution boundary, then reaches guest IRQ12 bytes | VM AUX guest smoke and DOS mouse smoke. |

## Explicit transfer

The selected protocol does not include wheel identification through sample-rate
handshakes, scaling effects, remote/read-data, RESEND/error timing, capture
mode, or host capture. These remain the existing
[`TODO(Medium) Advanced 8042 AUX protocol`](../../states/TODO.md), whose
admission requires a selected device corpus, manual command/report contract,
state/failure publication rule, and focused guest-visible proof. S4 neither
claims nor changes those rows.

## Similar-issue sweep

The sweep covered every AUX command and pending-parameter branch, `D4h` route,
report encoder and FIFO capacity check, per-origin IRQ refresh/deassert,
command-byte and interface gates, reset/finalize, machine/session mouse ingress,
focused AUX/guest/DOS tests, profile timing, and the advanced-AUX TODO. All
selected publication shares the existing single FIFO and deterministic advance
owner. No controller, keyboard, or host ABI change is required.

## Verification

- `core-machine-kbc-aux-port-smoke` covers selected commands, parameters,
  delay, IRQ12 gating, packet clipping, full-FIFO no-publication, reset, and
  finalize source release.
- `vm-kbc-aux-guest-smoke` and `vm-mouse-driver-dos-smoke` retain copied
  execution-boundary and guest-consumer proof.
- Full current-gate and documentation governance are required before S4
  acceptance.
