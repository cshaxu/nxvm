# T511 S8 8272A FDC Phase-Consumer Audit

`M5:T511:S8:FDC-PHASE-CONSUMER:NO-HIT`

## Sources

Intel *8272A Floppy Disk Controller* (order 210608-001, 1982), owner-managed
copy `assets/manuals/controllers/intel/210608-001_8272A_Floppy_Disk_Controller_1982.pdf`,
defines command, execution and result phases; MSR RQM/DIO/NDM observation;
non-DMA byte handshakes; DMA DRQ; interrupt/result acknowledgement; and reset
Sense Interrupt results.  The local NEC uPD765A manual corroborates the family
contract.  86Box, MAME, PCjs, Bochs and QEMU are comparison-only Other-L2
references where their selected implementation agrees; no external code is
copied and no controller duration is inferred from them.

## Owner/Consumer Matrix

| Boundary | Sole production path | Disposition |
| --- | --- | --- |
| Command to execution | `fdc.c` accumulates exactly one command, then schedules `PENDING_COMMAND`; its owner alone decodes and begins execution. | Command bytes cannot authorize a second command or data phase. |
| DMA byte service | FDC withdraws DRQ after every transferred byte, schedules the next byte gate, then publishes a new DREQ through the bound 8237A channel. | Each byte needs its own FDC edge and DMA grant; ATA PIO's sector rule is not applied. |
| Non-DMA byte service | The FDC's MSR exposes RQM/DIO/NDM only after its owner-local byte gate; data read/write consumes one byte then schedules the next gate. | A previous RQM does not authorize the successor byte. |
| Terminal completion/result | The final byte enters `PENDING_COMPLETE`; owner advance sets the result phase, publishes IRQ6 and drains result bytes through one result index. | Result acknowledgement withdraws only that completion; later commands require a new completion. |
| Reset/seek completion | DOR reset cancels execution/DRQ/IRQ; reset/seek pending state publishes a new Sense Interrupt result only at its due owner boundary. | Reset cannot leak an old transfer or let a stale IRQ acknowledge a new one. |
| Firmware and VM | INT 40h configures DMA2, waits for the result-phase MSR and drains the seven result bytes; VM only supplies frozen topology/media. | No firmware instant-completion shortcut or VM FDC-state mirror exists. |

## Executed Proof And Conclusion

The focused Debug cohort passed: `core-machine-fdc-smoke`,
`core-machine-fdc-topology-port-smoke`, `core-machine-fdc-media-change-port-smoke`,
`vm-fdc-port-smoke`, `vm-fdc-t242-corpus-port-smoke`,
`vm-model40-fdc-s24-smoke` and `vm-fdc-authority-smoke`.  It covers command and
result phases, RQM/DIO/NDM non-DMA transfer gates, repeated DMA byte gates,
IRQ6/Sense Interrupt acknowledgement, reset/media cancellation, real firmware
DMA/result consumption and the Model-40 board variant.

No ATA-style stale observation exists.  The FDC distinguishes every command,
data byte and result phase in its sole state owner; no production change is
warranted.
