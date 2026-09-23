# M5 T496 S6 IBM 5160 DOS Boot Convergence Ledger

`M5:T496:S6:XT-BOOT-LEDGER:COMPLETE`

## Scope And Completion Predicate

The selected IBM 5160-268 BYOB route must reach a DOS prompt, the DOS date
input checkpoint, or an installer guest-input page in Turbo without treating a
fixed wall-clock timeout as a success. For installation media, that interactive
installer page is the accepted startup terminal. Firmware and guest media
remain external runtime inputs. This ledger is exhausted only when every row is
either proved by that semantic checkpoint and its focused regression, or
transferred through a named subsequent S. No later checkpoint disposes of an
earlier row implicitly.

## Evidence Hierarchy

IBM's 5160 Technical Reference is the board wiring authority. Intel 8088,
8237A, 8253, 8259A and 8255A documentation and NEC 8272A-compatible source
are the component authorities. The already completed T487--T496 source/List-1/
List-2 records retain the visual-source and OCR boundaries. External emulator
implementations are research-only cross-checks: they can identify a candidate
relationship, never establish a normative behavior or a compatibility bypass.

## Frozen Boot-Chain Universe

| Batch | Required route and checkpoint | Current owner/evidence | S6 disposition |
| --- | --- | --- | --- |
| B1 | Reset, ROM mapping, 8088 execution, conventional/absent memory reach firmware POST | Core machine plan, CPU and memory routes; T484/T495 evidence | Retain; include in whole-chain replay and select only if a deterministic first failing owner is found. |
| B2 | PIT channel 0 drives PIC IRQ0 and an interrupt-enabled wait can resume | Core PIT, PIC and scheduler; T488/T490 ledgers | Reconcile as one signal chain, not two independent port tests. |
| B3 | PIT channel 1 requests DMA channel 0 refresh; DMA terminal/status lifecycle is visible to firmware | XT board wiring plus Core DMA/PIT/scheduler; T489/T490 ledgers | First repair candidate: verify the complete channel-1-to-channel-0 path, including reset, request, service and status-read semantics. |
| B4 | XT keyboard device completes BAT/scan delivery only through PPI latch and IRQ1 | Core XT keyboard and PPI; T491/T496 evidence | Retain; one-owner keyboard/PPI boundary is already source-backed but needs boot replay proof. |
| B5 | 8272A command/DRQ/DMA2/IRQ6/media path supplies the boot sector | Core FDC/DMA/PIC/media and XT profile; T492 evidence | Retain; inspect as a complete transfer route if B3 does not explain the first checkpoint failure. |
| B6 | CGA state and copied display snapshot expose POST and DOS semantic checkpoints | VADP sole owner and VM snapshot consumer; T493 evidence | Observation-only consumer; no second text/BIOS state or renderer inference. |
| B7 | Firmware boot policy reaches the selected floppy hand-off after POST | VM BYOB/session composition; T484/T495 evidence | Retain; firmware is a consumer, never an implementation target. |
| B8 | Scheduler, deadline and CPU interrupt hand-off preserve the order of B2--B5 while CPU is running or halted | Core scheduler/transaction/interrupt path; T469/T470 and XT unit tests | Inspect complete shared owner batch with B3 before a production change. |

## Reconciliation And First Repair Selection

The static owner sweep covers `src/core/machine/`, the selected XT profile,
and `tests/core` plus `tests/machine`. It confirms that the PIT output callback,
DMA request binding, request service, time/deadline publication and CPU
interrupt re-entry are all Core-owned. The XT profile supplies only frozen
construction values. No VM or firmware path owns any part of refresh.

The current B3 implementation surface exposes a source-backed board mismatch:
the selected XT contract requires PIT counter 1 to request **DMA channel 0**,
whereas the obsolete Core binding used channel 1. The finite repair is therefore
B3 plus the B8 scheduler hand-off, not a controller rewrite: construction
validation, channel-0 binding, PIT edge request polarity, DMA service/terminal
status and the blocked/deadline interaction must be reconciled together. B1,
B2 and B4--B7 have explicit next-S receivers and are not silently accepted.

## Planned S Sequence

1. **S6: ledger and batch selection:** this record fixes B1--B8 and selects
   the first source-backed batch without changing behavior.
2. **S7: PIT1/DMA0/scheduler batch:** audit and, if the full batch proves
   defective, repair its sole Core/XT-board route, remove any obsolete binding,
   and add focused reset/request/service/status regressions.
3. **S8: real floppy transfer batch:** only if S7 does not yield the DOS
   checkpoint, reconcile the complete 8272A/DMA2/IRQ6/media path before one
   owner-local repair.
4. **S9: remaining first-failure batch:** only if the actual replay assigns
   B1, B2, B4, B6, B7 or B8; its packet freezes the whole named batch before a
   repair.
5. **S10: corpus and closure:** replay the admitted compatible DOS images to
   prompt/date input, perform the cross-batch sweep, full gates, release build
   and dual-role closure review.

`M5:T496:S6:XT-BOOT-BATCH:SELECTED=B3+B8`

`M5:T496:S6:XT-BOOT-REPAIR:NO-SPECULATION`
