# T374 S4: Model-339 FDC Functional Contract Audit

## Source And Scope

The selected Model-339 path owns an 8272A-compatible floppy controller,
IRQ6/DMA2 route and an aftermarket 1.44 MB drive compatibility option. It is
not an assertion that such a drive shipped in a Model 339, and it introduces no
ROM or guest media. The controller-level source used for this audit is NEC's
[uPD765 application note](https://www.bitsavers.org/components/nec/_dataSheets/uPD765_App_Note_Mar79.pdf).
It establishes command/result phases, `Specify`, `Sense Drive Status`,
`Seek`, `Recalibrate`, `Sense Interrupt Status`, data-transfer commands,
invalid-command response, and the ready-line attention protocol. The
[uPD765A data sheet](https://hxc2001.com/download/datasheet/floppy/thirdparty/FDC/NEC/uPD765A.pdf)
remains the component-local interface source recorded by T372.

No external implementation, firmware, trace, media image or controller timing
was imported. The audit deliberately does not convert controller clocks,
head-load/unload fields, DMA handshakes, drive mechanics or Ready transition
latency into NXVM time.

## Contract-To-Owner Comparison

| Surface | Source contract | Current `core_machine_fdc` disposition |
| --- | --- | --- |
| Command/result phases and MSR | Command bytes and all result bytes use the data register; MSR RQM/DIO provides the command/result handshake. | `core_machine_fdc_command_length()`, `core_machine_fdc_msr()`, `core_machine_fdc_write_data()` and `core_machine_fdc_read_data()` own the phased state. Focused smokes cover normal/error result consumption. No timing value is inferred. |
| Specify and drive status | `Specify` stores controller parameters and DMA/non-DMA selection; `Sense Drive Status` returns selected drive status without an execution phase. | `CMD_SPECIFY` and `CMD_SENSE_DRIVE_STATUS` retain parameters and return status at the FDC owner. Existing port smokes cover non-DMA configuration and selected-drive status. |
| Seek, recalibrate and Sense Interrupt | Seek/recalibrate complete through an interrupt and must be consumed with `Sense Interrupt Status`; the latter returns ST0 and present cylinder. | The dispatcher, `core_machine_fdc_complete_simple()` and `CMD_SENSE_INTERRUPT` implement the IRQ6/state/result route. Existing media-change smoke consumes both recalibration completions. Seek mechanical duration remains later timing work. |
| Read/write/read-track/format | The controller supplies command/result state and DRQ/INT protocol; media and drive signals determine execution outcome. | The FDC owns command/result state, DRQ request/deassert and IRQ6 publication. Existing media and topology smokes cover DMA/non-DMA, normal/error transfers, format, cancellation and reset. Sector layout and physical transfer cadence remain separate storage/timing receivers. |
| Invalid command | The command table defines invalid codes as a one-byte ST0 `80h` result. | The default dispatcher produces `80h`; no repair selected. |
| Ready-line attention | After `Specify`, the controller scans drives; a Ready transition produces an attention interrupt. The following `Sense Interrupt Status` reports the affected drive not-ready state. | `core_machine_fdc_refresh()` detects only media-generation change and sets DIR disk-change state. It never calls the existing IRQ/`flagINTR` owner, so a visible ready transition lacks the documented attention/Sense-Interrupt path. **Select this as the next owner-local repair.** |

## Selected Repair Contract

The next packet may add a per-drive observed-ready state within
`core_machine_fdc` and, only on a transition of the existing logical
ready predicate, publish the existing IRQ6/`flagINTR` path with an ST0
not-ready attention result. `Sense Interrupt Status` must consume that state
and deassert the existing PIC source. A media-generation change without a
Ready transition remains only the already-owned DIR disk-change indication;
it must not manufacture an attention interrupt. DOR reset/cancel must clear
the pending attention through the existing reset owner.

This is a controller-state repair, not an assertion about physical signal
polarity, drive settling, motor speed, index pulses, DMA grant/service, board
waits or interrupt phase. Those remain exclusive inputs to the queued 5170
board/device phase-timing closure.

## Owner And Similar-Issue Sweep

The sweep inspected all dispatcher cases, command/result transitions,
`core_machine_fdc_raise_irq()`, `core_machine_fdc_refresh()`, DOR reset,
DMA terminal/cancellation, media-change helpers, the core machine advance
caller, and VM FDC topology/authority tests. The single absent mechanism is
ready-transition attention publication. Existing seek/recalibrate, transfer
completion and reset IRQ paths are not duplicated; the repair must reuse them.

Read Deleted/Write Deleted and Scan commands are listed by the uPD765 source
but not represented by the current generic media metadata/compare contract.
They are not silently converted into this repair and remain explicit later
functional-completeness dispositions. The controller's physical drive
interface and service durations remain timing work.

## Focused Baseline Replay

Git Bash invoked the installed Visual Studio Build Tools CMake executable,
which in turn invoked the already-configured Ninja generator, to build and run
the unchanged focused targets. Observed markers:

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T291:S1:FDC:PORT:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

The baseline replay validates the existing surface only; it does not satisfy
the selected attention-IRQ repair or declare FDC/5170 functional completion.
