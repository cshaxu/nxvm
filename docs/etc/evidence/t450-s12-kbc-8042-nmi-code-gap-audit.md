# T450 S12 KBC 8042/NMI Current-Code Gap Audit

## Scope And Method

This audit consumes, without changing, all 16 rows in the T450 S11
[KBC source checklist](../research/t450-s11-kbc-8042-nmi-function-timing-checklist.md).
The reviewed owners are `src/core/machine/kbc.c`, `kbc.h`, `machine.c`,
`machine_scheduler.c` and the selected board binding. Focused proof is
`tests/machine/core_machine_kbc_controller_smoke.c`,
`core_machine_kbc_serial_cadence_smoke.c`, `core_machine_kbc_aux_port_smoke.c`,
keyboard-host-ingress, PC/AT topology/composition and Model-339 clock smokes.
Every nonconforming row transfers once to the queued
[Core KBC 8042 and NMI phase contract](../../proposals/m5-core-kbc-8042-nmi-phase-contract.md).
No source or test is modified by this audit.

T464 S1 rechecked every row against the checklist's rendered Intel/IBM PDFs
and the recorded 86Box, Bochs, PCjs, MAME and QEMU references.  The audit's
former bare `L3` labels mean `Manual L3` only where the original selected
manual defines the relation.  A later-model AUX/PS2 extension is `Other L3`
only under an explicit profile; unsupplied Core-time conversion and physical
serial details are `fallback to L2`, never inferred from a behavioural model.

## Row Dispositions

| S11 ID | Current owner and observed behavior | Current proof | Disposition and unique receiver |
| --- | --- | --- | --- |
| KBC-R1 | `kbc.c` owns one logical guest FIFO, command-response delay and a separate native serial queue. OBF is the FIFO head and reading 0060h dequeues it; input-buffer-full is set and cleared synchronously during each write, so it is not a UPI firmware-processing state. | Controller and serial-cadence smokes cover FIFO, OBF and ordered delayed response. | Partial Manual L3: logical handshake exists; one DBBIN/DBBOUT/IBF service phase is missing. Receiver: T464 KBC phase contract. |
| KBC-R2 | `core_machine_kbc_read_status` supplies OBF/IBF/system/command-data/keyboard-enable/AUX bits at 0064h and `register_ports` owns 0060h/0064h. It does not model AT parity, receive-timeout, transmit-timeout or physical inhibit-switch status bits. | Controller, serial and host-ingress smokes cover data/status/command selection. | Partial Manual L3: selected ports and principal flags conform; sourced status bits are missing. Receiver: T464 KBC phase contract. |
| KBC-R3 | `core_machine_kbc_write_command` implements 20h, 60h, AAh, ABh, ADh/AEh, D0h/D1h and pulse-reset behavior; it adds PS/2 AUX commands A7h--A9h/D4h. AC, C0 and E0 are absent, and F0h--FFh only produces a reset request when bit 0 is selected, not a timed port-bit pulse. | Controller and AUX-port smokes cover retained command, reply, interface and pulse-reset paths. | Partial Manual L3: diagnostic/input/test commands and output-port pulse state are missing. The 6-us width remains L4; consumer delivery is board L3 or fallback to L2. Receiver: T464 KBC phase contract. |
| KBC-R4 | `command_byte`, `output_port`, `pending_write`, `keyboard_enabled` and callbacks are one controller owner. Command-byte bit layout retains IRQ1/IRQ12, disable and translation extensions rather than the exact AT reserved/PC/inhibit layout; board input/test ports are not represented. | Controller smoke covers command-byte, interface enable and output port. | Partial: one state owner exists; exact AT command/input/test-port bits are missing. Receiver: queued KBC phase contract. |
| KBC-F1 | `machine_scheduler.c` supplies plan-derived KBC ticks to `core_machine_kbc_advance`; that function advances configured serial, reply and typematic counters but has no UPI instruction, oscillator or four-to-seven-clock interrupt model. | Serial-cadence and Model-339 clock smokes prove rational input and selected synthetic cadence. | Partial L2: one time consumer exists; selected controller-clock/firmware cadence is unallocated. Receiver: queued KBC phase contract. |
| KBC-F2 | Native keyboard bytes pass through the one `core_machine_kbc_submit_native_bytes` boundary, a serial queue and optional Set-2-to-Set-1 translation. The code assumes already-valid bytes; it does not model 11-bit serial frames, parity checking, FFh errors, two-ms receive timeout or receive-interface hold. | Controller, serial-cadence, host-ingress and keyboard mapper smokes prove byte ownership and translation. | Partial: byte-level route conforms; physical serial/error behavior is missing. Receiver: queued KBC phase contract. |
| KBC-F3 | Keyboard commands, ACK/resend, scanning, typematic and selected scan sets are emulated in `core_machine_kbc_handle_keyboard_command`, with configurable abstract reply/serial ticks. It has no AT 15/2/25-ms timeout formula, clock waveform or complete keyboard-device source corpus. | Controller and serial-cadence smokes cover selected commands, responses, typematic and delayed reply ordering. | Partial: retained command path works; exact device timing and complete protocol are L2. Receiver: queued KBC phase contract. |
| KBC-F4 | `core_machine_kbc_refresh_current_irq` is the only keyboard IRQ1 publisher; FIFO-head origin and command-byte enable decide assertion, and dequeue/finalize release it through the PIC source object. | Controller, host-ingress, PC/AT topology and composition smokes prove IRQ1 route and release. | Partial L2: logical route conforms; OBF-edge to PIC/CPU visibility phase is unallocated. Receiver: queued KBC phase contract. |
| KBC-F5 | `core_machine_kbc_apply_output_port` is the sole A20/reset route: it updates memory A20 and requests CPU reset; D1 persists state and F0h--FFh reset selection does not overwrite it. No six-microsecond output pulse or selected reset/A20 phase exists. | Controller smoke covers D0/D1, A20, persistent state and pulse-reset request. | Partial: logical board binding conforms; pulse and consumer phase are missing. Receiver: queued KBC phase contract. |
| KBC-F6 | KBC owns no NMI mask or NMI producer. `machine_board.c` owns parity/D4 NMI mechanisms and RTC port 0070h owns its mask; KBC output-port callbacks only serve configured board consumers. | PC/AT ownership/composition and D4 platform smokes prove separated owners. | Conforming negative ownership boundary. |
| KBC-F7 | No code converts UPI electrical reset, oscillator or bus/serial AC tables to Core ticks. | Source/owner inspection. | Correctly L4 excluded. |
| KBC-T1 | `core_machine_kbc_register_ports` installs the one 0060h/0064h provider at Core construction; no VM parallel port route exists. It models only those two ports, which is the relevant AT host interface. | Controller, VM keyboard and PC/AT topology smokes cover registration and composition. | Conforming selected topology. |
| KBC-T2 | IRQ1 is bound in `core_machine_kbc_bind_core_services`; the AUX IRQ12 route is explicitly conditional on an AUX-present profile and is not claimed as an IBM 5170 fact. | Controller/AUX, PC/AT topology and composition smokes prove both routes and absence handling. | Conforming IRQ1; AUX remains an explicit extension, not a 5170 conclusion. |
| KBC-T3 | `machine.c` binds KBC to Core memory and CPU execution, preserving one A20/reset receiver; Model-40 optionally binds a board-specific output-port callback. The AT pulse-to-consumer phase is not selected. | Controller, Model-40 and machine reset smokes prove owner-local binding/reset. | Partial L2: route ownership conforms; selected board/CPU phase is missing. Receiver: queued KBC phase contract. |
| KBC-T4 | No KBC code changes the NMI mask; board NMI mechanisms remain independent. Firmware power-on/POST policy is not emulated by KBC. | PC/AT ownership and board source inspection. | Conforming separation; firmware policy is L2 outside KBC. |
| KBC-T5 | T449's timeline/reset/observation path remains the sole scheduler; KBC consumes ticks and emits existing IRQ/A20/reset endpoints. No second KBC scheduler or mutable board state exists. | Scheduler inspection and clock/serial/lifecycle smokes. | Unallocated L2 input, not a duplicate-owner defect. Receiver: queued KBC phase contract. |

## Completeness, Minimality And Transfer

## T464 S1 Per-Row Tier Reconciliation

| S11 ID | Authoritative tier after cross-check | T464 receiver |
| --- | --- | --- |
| KBC-R1 | Manual L3 host-buffer semantics; service cadence is board L3 or fallback to L2. | IBF service phase. |
| KBC-R2 | Manual L3 selected status bits. | Source error/inhibit status state. |
| KBC-R3 | Manual L3 selected commands; pulse width L4 and consumer phase board L3 or fallback to L2. | AC/C0/E0 and pulse state. |
| KBC-R4 | Manual L3 command/input/test-port fields. | Exact AT fields and board input/test state. |
| KBC-F1 | Manual L3 UPI relation; selected cadence is board L3 or fallback to L2. | No invented instruction clock. |
| KBC-F2 | Manual L3 frame/error semantics; delivery cadence is board L3 or fallback to L2. | Error/receive-hold state. |
| KBC-F3 | Manual L3 protocol/timeout semantics; exact endpoint coverage is board L3 or fallback to L2. | Status/error boundary only. |
| KBC-F4 | Manual L3 IRQ1 route; PIC/CPU observation phase is board L3 or fallback to L2. | Preserve sole publisher. |
| KBC-F5 | Manual L3 reset/A20 binding; pulse width is L4 and consumption phase is board L3 or fallback to L2. | Preserve sole output owner. |
| KBC-F6 | Manual L3 negative NMI ownership boundary. | No KBC NMI path. |
| KBC-F7 | L4 electrical exclusion. | No Core tick derivation. |
| KBC-T1 | Manual L3 5170 port topology. | No alternate port route. |
| KBC-T2 | Manual L3 IRQ1; AUX is Other L3 only for selected later profiles. | Keep AUX profile-gated. |
| KBC-T3 | Manual L3 logical board binding; delivery is board L3 or fallback to L2. | Preserve one A20/reset receiver. |
| KBC-T4 | Manual L3 NMI separation. | No KBC NMI path. |
| KBC-T5 | board L3 when selected; otherwise fallback to L2. | Consume no host-time source. |

All `KBC-R1`--`KBC-R4`, `KBC-F1`--`KBC-F7` and `KBC-T1`--`KBC-T5` rows have
one disposition. The later receiver must refine the current KBC owner and
consume T449's clock/timeline route. It must not add a second FIFO, KBC clock,
0060h/0064h adapter, IRQ publisher, A20 store, reset route or NMI mechanism.
The bounded gaps are exact UPI/AT buffer/status/command state, serial error and
device timing, selected controller/IRQ/A20 phase, and explicitly sourced AUX
extensions.
