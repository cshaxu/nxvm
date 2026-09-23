# T348 S2 DMA Port, Page, And Transfer Layout

## Shared Mechanism Repair

`IncreaseCurrAddr` and `DecreaseCurrAddr` are the sole post-transfer address
update helpers for normal transfers and the memory-to-memory path. They now
advance or decrement only the 16-bit controller address. The PC/AT page latch
is programming state for the current 64 KiB byte-channel or 128 KiB word-
channel block; it is not a carry/borrow extension. This removes the S1
reproduced `1FFFFh -> 20000h` byte-channel crossing and applies equally to
normal, decrement, word-channel, and memory-to-memory callers.

The same S2 source/test sweep reproduced a second common construction defect:
`dma_controller` selected the peer controller only at `C0h` and above, so PC/AT
secondary page ports `89h--8Fh` silently wrote primary page storage. It now
recognizes that board-owned page-port range as secondary-controller state.

## Permanent Owner-Smoke Matrix

`core-machine-dma-channel-smoke` now proves:

| Contract | Permanent proof |
| --- | --- |
| Primary ports | Flip-flop-cleared channel-0 address/count pair readback, existing channel-2 programming/control/mode coverage, and page ports `81h/82h/83h/87h`. |
| Controller controls | Both controller command, request, single-mask, all-mask-clear, and master-clear routes (`08h--0Fh`, `D0h--DEh`) are decoded into independent primary/secondary state after an explicit reset. S3 owns the resulting request, priority, and cascade policy. |
| Secondary sparse ports | Channels 5--7 are programmed through `C4h/C8h/CCh` plus sparse count ports; each address/count pair reads back through the same port shape and performs a real device-to-memory word transfer. |
| Secondary page ports | `89h/8Ah/8Bh/8Fh` read back their peer-controller pages with the required low-bit masking. |
| Byte page wrap | Channel 2 increments `FFFFh -> 0000h` and decrements `0000h -> FFFFh` while page `01h` remains fixed; the second byte appears at `10000h` or `1FFFFh`, never a neighboring page. |
| Word page wrap | Channel 5 increments and decrements its word address across `FFFFh/0000h` while page `02h` remains fixed; the two words appear at `3FFFEh` then `20000h`, or the inverse, never a neighboring 128 KiB block. |
| Retained behavior | Primary demand/single/block, masking/deassertion, auto-init, decrement, terminal count, and memory-to-memory checks remain in the same owner smoke. |

The byte and word address forms intentionally remain distinct: byte addresses
form `page << 16 | address`, while word channels form `page << 16 | address <<
1`; the PC/AT word-page low bit is not addressable. No abstraction erases that
hardware layout distinction.

## Remaining Owners

S3 retains request/mask/priority/cascade behavior, software versus hardware
EOP, and channel-4 isolation. S4 retains physical transaction atomicity,
reset/finalization, FDC/ATA/PIC visibility, and timeline reconciliation. The
later L3 bus-timing candidate alone may add physical grant-duration or general
wait-state behavior.
