# T503 S6 FDC, DMA2, PIC And Firmware Route Audit

`M5:T503:S6:FDC-FIRMWARE-ROUTE:OK`

## Sources and boundary

The normative controller source is NEC *uPD765A/uPD765B Floppy-Disk
Controller*, NECEL-000324.  Its searchable scan was used only to locate pages;
the following source pages were rendered and visually checked: printed pp.
5-3--5-5 (DRQ, DACK, TC, INT, RESET and READY), 5-7 (A/B differences) and
5-8--5-10 (DMA and terminal-count waveforms).  IBM *5160 Technical Reference*
remains the board authority for the `3F2h`/`3F4h`/`3F5h`, DMA2 and IRQ6 route
already frozen in List 1.

The NEC reset text specifies when READY produces an interrupt but does not
spell out the PC-compatible number and ordering of pending Sense-Interrupt
reports.  Local reference implementations agree on the selected observable
sequence: Bochs `iodev/floppy.cc` counts four reports after reset; 86Box
`src/floppy/fdc.c` decrements `reset_stat` through drives 0--3; PCjs
`machines/pcx86/modules/v2/fdc.js` records IBM 5170 firmware draining four
ordered responses.  This is **Other L3** corroboration for report cardinality,
not a replacement for NEC or IBM authority.  MAME and QEMU FDC source are not
present in the approved local research set, so no claim depends on them.

## Complete route disposition

| Route stage | Sole owner and route | Evidence and disposition |
| --- | --- | --- |
| Command and DOR | `core_machine_fdc` owns ports, phase state and reset-edge detection. | NEC main-status/data grammar and IBM selected ports: Manual L3. |
| Reset completion | `core_machine_fdc_schedule_reset_completion` owns the 1.024-ms deadline and the ordered four-report pending state. Its sole input is the copied frozen `ready_mask`. | NEC supplies the reset/READY condition and time; Bochs, 86Box and PCjs corroborate four reports: Manual L3 input/timing plus Other L3 cardinality. |
| Data request | `core_machine_fdc_request_assert/deassert` invokes the copied, Core-issued DMA binding. `machine_board.c` validates the binding and is the only path to DMA2. | NEC DRQ/DACK/TC relationship; IBM DMA2 map: Manual L3 interface. |
| Transfer completion | The DMA2 provider calls only FDC byte and terminal handlers; FDC alone leaves execution, constructs results and releases DRQ. | NEC TC terminates Read/Write/Scan: Manual L3 logic order. |
| Interrupt and firmware | FDC's bound PIC source is IRQ6. Result/Sense consumption clears the same source; guest firmware receives it only through PIC/CPU dispatch. The Model-40 observation is copied terminal data, not a command or state path. | NEC INT and IBM IRQ6 route: Manual L3 interface. |
| Media and mechanics | The media provider owns bytes, geometry and insertion generation; FDC owns no image cache. Drive mechanics, analog PLL, raw CRC/address marks and host I/O remain outside this logical-media route. | Existing List-1 external boundaries; no invented timing. |

The only defect found in the direct audit was documentary: T492 S3/S4/S5 and
one FDC header comment still described an earlier ready-sampled reset queue.
They are corrected here and in place.  `ready_mask` is the single frozen board
input demanded by NEC; it replaces the implicit always-ready assumption without
creating a profile-side controller, firmware shim, polling loop or second FDC
state.  Selected profiles explicitly supply their proven all-ready wiring.

## Proof

`core-machine-fdc-smoke` and `core-machine-fdc-topology-port-smoke` each
exercise reset with media absent from at least one controller select and prove
the ordered `C0h`--`C3h` Sense-Interrupt reports, the single IRQ6 source and
the subsequent invalid response.  The focused route sweep also includes
DMA-boundary, media-change, XT/5170 and Model-40 FDC tests.  T503 S6 repeats
the complete repository unit suite before acceptance.
