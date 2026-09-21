# Selected Controller And Board Qualification

## Goal

Qualify only the frozen Standard and PC110 component set through one shared
Core state/time architecture. Read the selected board before assuming its
RTC, keyboard, video or storage chip identity.

## Scope And Batches

The fixed-product proposal first supplies the accepted BOM and retirement map.
For each distinct retained/new controller, admit its own bounded task: source
collection; exhaustive function/timing List 1; current-code gap List 2;
primary-manual/emulator cross-check; complete owner repair; downstream wiring
and product regression. Do not repeat one task per machine when the same chip
contract is shared, or combine unrelated chips into an unbounded repair task.

PIC, DMA, PIT, RTC, KBC/keyboard, FDC, video and fixed-disk are inventory
categories, not assumed chip identities. Include actual board glue, clock,
refresh, IRQ/DRQ, memory decode, firmware and reset dependencies in each ledger.
Any PC110 chipset subfunction reusing Intel semantics still needs proof for
its routing/extensions. External timing inputs can satisfy a chip contract;
unknown internal timing cannot become L3 merely by accepting a number.

## Exit And Boundaries

One device/port/memory/deadline owner; no BIOS-specific responses, parallel
scheduler, second framebuffer or generic-ATA substitution. Exact-source
values/formulas are L3, model/ratio estimates L2, order-only behavior L1.
Run complete unit and required board/integration regressions and account for
every List 1/List 2 row before claiming a controller complete.
