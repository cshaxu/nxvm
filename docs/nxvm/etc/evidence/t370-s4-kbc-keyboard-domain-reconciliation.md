# T370 S4: 8042 And Keyboard Domain Reconciliation

## Decision

The selected Model-339 route has one controller-owned guest-visible path.
`kbc.c` owns ports `60h` and `64h`, command and keyboard state, the one
origin-tagged FIFO, delayed-response publication, and the IRQ1/IRQ12 source
handles.  Keyboard command replies, scans and typematic bytes enter that same
FIFO as keyboard origin; controller replies retain controller origin; the FIFO
head alone determines which eligible KBC source is asserted.  `pic.c` owns
logical IRQ selection and acknowledgement.  Reset clears the KBC state while
preserving configured fields, releases both sources, and finalization releases
both sources again.

The IBM PC/AT references establish the controller/keyboard protocol, including
controller self test and keyboard command behavior.  The Intel UPI-41/42 manual
provides 8042 host-interface context.  They do not label a conversion from an
8042/keyboard oscillator or serial exchange to the project's completed-core
elapsed ticks.  The Model-339 descriptor has KBC clock ratio `1/1` relative to
those project ticks, but its `kbc_typematic_initial_ticks`,
`kbc_typematic_repeat_ticks`, and `kbc_command_response_ticks` are each zero.
The ratio is deterministic callback scheduling, not a physical clock, and the
three zero values explicitly leave core-generated typematic and deferred
command-response duration disabled.  No timing scalar or runtime change is
admitted.

Primary sources:

- [IBM PC/AT Technical Reference, March 1984](https://ftp3.us.freebsd.org/pub/misc/bitsavers/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf), keyboard-controller initialization and keyboard commands.
- [IBM PC/AT Technical Reference, September 1985](https://www.bitsavers.org/pdf/ibm/pc/at/6139362_PC_AT_Technical_Reference_Sep85.pdf), controller initialization, self test and keyboard interface behavior.
- [Intel UPI-41/42 User's Manual](https://www.ceibo.com/eng/datasheets/Intel-8041-Manual.pdf), 8042-family host-interface context.

## Route And Domain Matrix

| Boundary | Existing owner and retained proof | S4 domain disposition |
| --- | --- | --- |
| Host command/data acceptance | `kbc.c` owns port `60h` data and port `64h` command writes, pending parameter state, command-byte interface gating and output-port callback. T351 S2/S3 controller smoke covers self test, enable/disable, command byte and selected keyboard commands. | Protocol ordering is retained. Input-buffer processing, host pacing and serial exchange duration have no project-domain conversion. |
| FIFO publication and IRQ visibility | `kbc.c` enqueues tagged output once, refreshes the current FIFO-head source, and deasserts the prior source on read; keyboard head may assert IRQ1 and AUX head IRQ12 only when their command-byte and interface gates allow it. T351 S5 proves mixed FIFO order and source release. | FIFO/IRQ order is logical state publication. PIC acknowledgement and physical IRQ/INTA propagation remain separate owners and phase work. |
| Command response delay | `core_machine_kbc_schedule_response_byte` retains a KBC-owned pending response until capacity permits; `core_machine_kbc_advance` consumes `command_response_ticks`. Focused controller and AUX smokes exercise nonzero fixture values and zero-delay publication. | Model 339 selects `0`, so no deferred duration is scheduled. A fixture value demonstrates the mechanism only; it is not a board timing result. |
| Keyboard scan and typematic | `core_machine_kbc_submit_scan_code` gates scanning, starts repeat only when both configured values are nonzero, and `core_machine_kbc_advance` publishes repeats through the same FIFO. T351 S3 and controller smoke cover command/default/cancellation and fixture timing. | Model 339 selects both values as `0`, so core-generated typematic remains disabled. Keyboard protocol configuration is not a conversion to physical repeat time. |
| Peripheral callback and trace/copy boundary | `machine.c` advances the KBC clock before VADP in the one peripheral callback and records `KBC_ADVANCE`; profile selection copies the three configuration fields into one core-machine config. Existing timeline and KBC smokes cover deterministic ownership. | `1/1` is relative to completed core elapsed ticks, while trace records order and supplied tick count only. Neither is a measurement of controller latency or a copied host-input timestamp. |
| Reset/finalize | KBC reset preserves configured fields, clears FIFO/pending response/typematic state, applies default output state and deasserts IRQ1/IRQ12; finalization releases both sources. T351 S5 proves queued output cancellation and re-publication after reset. | Cancellation is logical and deterministic. No reset settling interval or device-clock duration is admitted. |

## Sweep And Source Limit

The S4 sweep reviewed all KBC command/data/FIFO/IRQ1/IRQ12/advance/reset/
finalize paths in `kbc.c`, KBC clock-domain creation and peripheral callback in
`machine.c`, Model-339 descriptor and contract copying in the default PC/AT
profile, KBC trace consumers, and the controller, keyboard, AUX, lifecycle and
timeline focused smokes.  The routes preserve one KBC state owner, one FIFO,
one callback and one copied configuration boundary; no parallel scheduler,
unreleased source or second publication queue is present.

86Box, MAME and PCjs are not numeric sources for this result.  The source
prerequisite is absent: a selected 8042/keyboard clock and a board-labelled
mapping from it to a project elapsed domain.  They may only become qualitative
cross-checks after a primary board/controller contract supplies that missing
boundary, and cannot make the Model-339 `0` fields physical timing values.

## S5 Receiver

S5 receives the selected planar-parity NMI and CGA/VADP visible-state/raster
ledger, followed by the T370 task-level transfer audit.  It must retain these
KBC physical transfers: 8042/controller oscillator conversion, keyboard serial
clock and ACK exchange, typematic/response latency, input-buffer service time,
IRQ propagation and INTA waveform.  It must not claim that T370 or the 5170
Model-339 has reached L3 merely because the logical owners are reconciled.
