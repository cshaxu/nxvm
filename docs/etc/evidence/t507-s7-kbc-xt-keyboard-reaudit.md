# T507 S7 KBC and XT Keyboard Re-Audit

`M5:T507:S7:KBC-XT-KEYBOARD:IN-REVIEW`

## Sources and Method

The source authority was read from rendered pages, not OCR alone:

- Intel *UPI-41A/41AH/42/42AH User's Manual* (October 1993), pp. 55--57,
  `assets/manuals/controllers/intel/231318-006_UPI-41A_42AH_Users_Manual_Oct1993.pdf`.
  The rendered bus-interface page confirms the DBBIN/DBBOUT separation, OBF
  clear-on-read, IBF on host write, and command/data selection.
- IBM *Personal Computer AT Technical Reference* (March 1984), System Board
  pp. 1-37--1-42, `assets/manuals/controllers/ibm/IBM_5170_Technical_Reference_1502243_Mar1984.pdf`.
  The rendered p. 1-38 diagram confirms the 8042's single system-side
  input/output buffers, keyboard wires, System Reset, and Gate A20.
- IBM *Personal Computer XT Technical Reference* (April 1983), System Board
  and keyboard pp. 1-5, 1-28, 1-65--1-67,
  `assets/manuals/controllers/ibm/IBM_5160_Technical_Reference_APR83.pdf`.
  It is the authority for the separate 8255/PPI, port-B keyboard lines,
  IRQ1, serial frame and reset/BAT relations.

Read-only comparison covered 86Box's `kbc_at.c`, `kbc_xt.c`,
`keyboard_xt.c` and `ppi.c`; Bochs `iodev/keyboard.cc`; and PCjs's PCx86
`keyboard.js`/`chipset.js`.  MAME and QEMU keyboard sources are not present
in the approved local reference corpus.  These implementations corroborate
the distinct AT and XT topologies and queue/IRQ ownership; they do not
override IBM or Intel, and no external code was imported.

## List 1: Source Contract

| ID | Source-qualified relation | Level | Sole NXVM owner/receiver |
| --- | --- | --- | --- |
| K1 | UPI DBBOUT is read through the data selection and the read clears OBF. | Manual L3 | `kbc.c` FIFO head and `0060h` dequeue. |
| K2 | UPI host writes set IBF and latch command/data selection; IBM maps data to `60h` and command/status to `64h`. | Manual L3 for handshake; L2 for firmware-service duration. | `kbc.c` input/write state. |
| K3 | IBM 5170 status exposes OBF, IBF, system, command/data and inhibit meanings. | Manual L3 | `core_machine_kbc_status`. |
| K4 | IBM command-byte, `20h`, `60h`, `AAh`, `ABh`, `ADh`, `AEh`, `C0h`, `D0h`, `D1h`, `E0h`, and pulse-output semantics are controller operations. | Manual L3 except ROM/RAM diagnostic dump and pulse width, which are L2. | `kbc.c` command/pending-write/output-port state. |
| K5 | IBM connects output-port bit 0 to System Reset and bit 1 to Gate A20. | Manual L3 causal route; pulse duration is L2. | `core_machine_kbc_apply_output_port` to memory/CPU reset request. |
| K6 | IBM 5170 keyboard delivery is IRQ1; the selected original board has no AUX mouse interface or IRQ12 route. | Manual L3 | Frozen 5170 profile selects KBC without AUX; `kbc.c` publishes only IRQ1. |
| K7 | XT uses the 8255 PPI at `60h`--`63h`, PB6 clock hold/PB7 clear, PA data and IRQ1; it is not an 8042. | Manual L3 | `xt_ppi_keyboard.c` plus `xt_keyboard.c`. |
| K8 | XT keyboard reset is clock-low for at least 12.5 ms; BAT returns after 300--500 ms; serial first edge and active phase are ranged. | Manual L3 inputs; macro conversion is L2. | `xt_keyboard.c` one reset/BAT/serial deadline producer. |
| K9 | Keyboard typematic and host-device serial details require the selected keyboard/controller firmware and board timebase. | L2 | Existing KBC clock-domain receiver; no host clock or guessed source is added. |
| K10 | KBC and XT keyboard reset/finalize cancel output and IRQ publication through their owning state. | Manual L3 causal reset; timing is L2. | Existing controller reset/finalize and Core scheduler. |

## List 2: Current Code and Repair Disposition

| ID | Current path and audit result | Disposition |
| --- | --- | --- |
| G1 | `kbc.c` alone owns `60h`/`64h`, output origin, OBF, command/data state, IRQ1/IRQ12, A20 and reset.  `machine_scheduler.c` asks it for one earliest event and advances it through the copied clock domain. | Retained single owner. |
| G2 | `xt_ppi_keyboard.c` owns the XT PPI latch, PA/PB/PC behavior, IRQ1 and NMI fault separation; `xt_keyboard.c` owns its 16-byte queue, reset/BAT/serial state and deadline. | Retained separate XT topology; no AT alias. |
| G3 | The pre-existing XT BAT-to-first-edge regression remains correct: an exact BAT deadline leaves the first serial edge pending. | Existing Manual-L3 relation retained. |
| G4 | The IBM 5170 root/Model 339 previously reused the generic default-AT AUX/IRQ12 route, exposing a later PS/2 extension as original-AT hardware. | **Repaired:** the IBM descriptor selects a four-route table and freezes `kbc_aux_absent`; generic default-AT keeps its distinct extension table. |
| G5 | Generic default-AT intentionally retains its optional AUX extension in the one KBC owner, including IRQ12; the IBM root does not select it. | Explicit immutable profile choice; no second controller or runtime inference. |
| G6 | DBBIN service duration, status error latches, diagnostic dump contents, keyboard electrical frames and exact output-pulse duration lack a selected controller ROM/board-time source. | Retained L2, with existing owner-local state and no fabricated deadline. |

## Proof and Simplicity

The repair removes the false IBM profile route without removing the generic
extension. It adds no controller state, port provider, callback, scheduler,
host-time input, or profile-time mutation. The IBM profile smoke verifies four real routes and
`kbc_aux_absent`; the Model-339 composition smoke verifies the constructed
Core has neither AUX presence nor AUX enablement. The generic default-AT
smoke and AUX guest smoke verify the retained extension. Focused KBC/XT/profile
tests pass 9/9 before the required complete unit replay.
