# T511 S9 HDC Personality Phase-Consumer Audit

`M5:T511:S9:HDC-PHASE-CONSUMER:NO-HIT`

## Sources

The retained T510 List 1/List 2 and source ledgers cover ATA-3, IBM WD1003,
Compaq/WD and IBM/Xebec XT.  They establish task-file/PIO, WD1003 and Compaq
interface, and Xebec DCB/DMA relations; their source-defined portions remain
Manual-L3.  86Box/Bochs/PCjs corroborate separate personality state machines
as Other-L2 only.  No external code or unqualified service duration is used.

## Owner/Consumer Matrix

| Personality/boundary | Sole production path | Disposition |
| --- | --- | --- |
| ATA PIO | `hdc.c` changes DATA_READ/DATA_WRITE to a pending-sector phase after exactly 512 bytes; service then republishes BSY-clear/DRQ. Default INT 13h waits before every sector. | T510 repaired the one real stale consumer; successor sectors require fresh DRQ. |
| WD1003/ST-506 | The explicit WD1003 task-file personality owns CHS, command, PIO buffer, status and IRQ14; `3F6h` is its fixed-disk selector, not ATA device control. | One PIO phase/IRQ owner; no ATA carry-over or stale status acknowledgement. |
| Compaq/WD 40 MB | The explicit Compaq personality owns its selection, PIO/status/IRQ14 and shared `3F7h` read contribution. | Its PIO phases are the same sole HDC state, without an FDC or VM status mirror. |
| Xebec XT | Xebec owns DCB collection, delayed command completion, block DMA3 request, sector transfer, response bytes and optional IRQ5. | Block DMA is intentionally sustained until block terminal/response; every response byte has a separate index and final acknowledgement. |
| Reset/status | Each owner path clears the same pending IRQ on status/result acknowledgement and clears PIO/DMA work on reset/finalize. | An old IRQ/DRQ cannot authorize a later command. |

## Executed Proof And Conclusion

The focused Debug cohort passed: `core-machine-hdc-smoke`,
`core-machine-xebec-wiring-smoke`, `core-machine-compaq-hdc-s5-smoke`,
`core-machine-compaq-hdc-machine-s5-smoke`, `vm-hdc-port-smoke`,
`vm-model40-hdc-s26-smoke` and `vm-rom-int13-hdd-type-smoke`.  Together with
T510's two-sector read/write and actual Windows Setup entry, it covers all
selected personality consumers.

No additional ATA-style stale observation exists.  The only hit was already
repaired in the default ATA firmware; no new production change is warranted.
