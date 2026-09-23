# T488 IBM 5160 8259A Closure Audit

`M5:T488:8259A-CLOSURE:ACCEPTED`

T488 followed the required independent-unit sequence before any repair:

| Stage | Complete result |
| --- | --- |
| S1 original source | Visually checked Intel 8259A and IBM 5160 primary-source ledger records pages, scan/OCR limit and selected XT topology. |
| S2 List 1 | 18 chip rows and 3 XT rows freeze all selected register, command, IRQ, acknowledge, reset, topology and timing-category rules. |
| S3 List 2 | Each frozen row maps to the existing `pic.c` state/command owner, immutable XT topology and focused test route. |
| S4 one owner | `pic.c` retains the sole state, port, producer-binding and logical-acknowledge route. The selected XT configuration registers only `20h`/`21h`; focused PIC and XT profile smokes pass. |

The full implementation batch is empty. This is a simplification result: no
wrapper, second controller, VM port filter, state mirror or compatibility path
was added. No production source, test, public ABI, firmware/media input or
release artifact changed.

Manual L3 covers the selected PIC state and logical order. The source does not
place delivery onto an elapsed Core time axis, so that placement remains L2;
electrical waveforms remain L4/out of scope. The next independent receiver is
the queued IBM 5160 8237A DMA unit; it must begin its own original-source
ledger and cannot reuse this PIC result as a DMA implementation authorization.
