# T488 S3 IBM 5160 8259A Current-Code Gap List 2

`M5:T488:S3:8259A-LIST-2:ACCEPTANCE-CANDIDATE`

All 18 Intel rows map to the sole `src/core/machine/pic.c` state/command
owner: IRR/ISR/IMR, ICW/OCW parsing, trigger/priority/mask/EOI/poll/cascade and
logical `scan`/`get` acknowledgement. T456's retained row-by-row source and
focused-proof matrix remains current evidence for every chip row; it has no
AT-only state inside the controller itself.

| XT List-1 row | Current production route | Disposition |
| --- | --- | --- |
| PIC-XT1 | Immutable XT profile selects `CORE_MACHINE_PIC_TOPOLOGY_SINGLE`; `core_machine_pic_initialize` registers master ports only. | Present; no slave alias or VM port filter. |
| PIC-XT2 | PIT/PPI/device sources bind only through `core_machine_pic_irq_source`; Core CPU consumes the same scan/get path. NMI bypasses PIC. | Present; producers retain their own units. |
| PIC-XT3 | `core_machine_pic_get_interrupt` is the one logical vector acknowledgement owner. | Present L3 sequence; elapsed placement stays L2. |

The complete List-2 batch is empty: source semantics and XT topology already
use one owner/path. No repair is admissible. The S4 audit must prove the
single-PIC port absence and retain electrical/elapsed boundaries.
