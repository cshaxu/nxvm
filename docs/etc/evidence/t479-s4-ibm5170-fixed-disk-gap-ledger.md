# T479 S4 IBM 5170-339 Fixed-Disk Current-Gap Ledger

`M5:T479:S4:IBM5170-GAP-LEDGER:OK`

This consumes every source-ledger row A1--A15. `Manual L3` means the named
guest-visible fact has a direct IBM source. It does not elevate unproven
physical service duration to L3.

| Batch | Current owner/path | Gap and required S5 result | Regression/proof | Disposition |
| --- | --- | --- | --- | --- |
| A1, A3--A5 | `src/vm/profile/default_profile/pc_at_profile.c` declares Model 339 with `hdc_present == false`; firmware/reset composition deliberately rejects a disk. | Replace that false frozen declaration with the source-proven one-drive IBM configuration, CMOS/parameter-table values after the exact drive type is confirmed. No runtime option or separate Model-339 variant. | Extend `vm_ibm_5170_model_339_composition_smoke` from no-HDC assertions to selected-HDC/CMOS/firmware assertions. | S5. |
| A2, A6--A12 | `src/core/machine/hdc.[ch]` is the sole mutable HDC owner; S3 config exposes only ATA and Compaq personalities over a common flat task-file shape. | Add one explicit IBM WD1003/ST-506 personality and its source-qualified command grammar through this same object. Retain media bytes/persistence in `src/vm/machine/hdd.[ch]`. | New focused IBM HDC smoke plus retained ATA/Compaq tests. | S5. |
| A7, A8, A12 | VM media provider owns sector bytes and geometry; current HDC normal PIO path owns CHS/data/IRQ state. | Validate selected geometry at construction and consume it only through the one HDC state machine. No second CHS truth or controller-local image cache. | Read/write/multi-sector and invalid-CHS focused tests. | S5. |
| A9 | Current command decoder is ATA/Compaq-oriented and firmware labels its PIO sequence as ATA. | Implement the eight IBM command families and their documented rejected-command path; rename generic firmware wording where it incorrectly describes all fixed disks as ATA. | Command/error/IRQ focused matrix, including every admitted opcode. | S5. |
| A10--A11 | Existing `1F0h`--`1F7h` and `3F6h` routing reflects retained ATA/Compaq contracts. | Implement IBM status-read IRQ acknowledgement and the IBM `3F6h` high-head bit inside the one IBM personality branch; do not inherit ATA alternate-status/device-control semantics. | Port-map, acknowledgement, high-head and reset tests. | S5. |
| A13, A15 | No MFM encoding, ECC/recovery, raw sectors, parity/NMI or board-phase owner exists. | Keep absent; do not widen HDC or VM media to fake it. | Static owner sweep. | `TODO` physical-media residual. |
| A14 | `core_machine_hdc_advance` exposes causal completion but does not consume a source-qualified IBM step-rate selector. | Add the manual step-rate selector and 1023-step bound to the immutable IBM personality/state transition path on the existing Core time axis. Do not manufacture rotation or drive-ready completion time. | Selector/extrema/reset tests and an explicit no-rotation-deadline assertion. | Manual L3 selector; L1 residual. |

## S5 Change Boundary

S5 is allowed to change only the Core HDC protocol/configuration, its one
state machine and port dispatch, the immutable Model-339 descriptor/firmware
binding, tests, and task evidence. It must delete any now-false no-HDC model
assertion and generic-ATA wording on the IBM route. It must retain:

- one `core_machine_hdc` mutable owner of registers, command phase, DRQ, IRQ
  and causal deadline;
- one VM media provider mutable owner of image bytes and persistence;
- construction-only copied profile data; and
- existing explicit ATA PIO and Compaq Model-40 personalities and their
  regression corpus.

The S4 page audit has fixed the command/control pages: printed pages 1--10.
If a later S5 row goes beyond those pages, it remains unavailable until a
primary page is recorded; it is not filled from an emulator or ATA behavior.
