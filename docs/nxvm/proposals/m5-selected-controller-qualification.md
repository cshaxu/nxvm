# Selected Controller And Board Qualification

## Goal

Qualify all retained XT/AT/DeskPro/default components and later PC110 through one shared
Core state/time architecture. Read the selected board before assuming its
RTC, keyboard, video or storage chip identity.

## Current Product Context

The completed fixed-product cutover selects one Profile per implemented XT, AT, DeskPro and
default product. Controller work starts from that construction: external BYOB
firmware is resolved by the selected Profile, NXVM.ini supplies only runtime
media/presentation, and deployment is only `assets/nxvm/<profile>/`. A repair
must not recreate a YAML/catalog machine selector or board-specific second
execution path.

## Scope And Batches

The fixed-product proposal supplies each retained BOM and migration map;
no current machine's hardware is retired by the structural cleanup.
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
