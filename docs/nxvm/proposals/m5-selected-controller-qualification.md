# Selected Controller And Board Qualification

## Goal

Qualify all retained XT/AT/DeskPro/default components and later PC110 through one shared
Core state/time architecture. Read the selected board before assuming its
RTC, keyboard, video or storage chip identity.

## Current Product Context

The completed fixed-product cutover selects one Profile per implemented XT, AT, DeskPro and
default product. Controller work starts from that construction: external BYOB
firmware is resolved by the selected Profile, NXVM.ini supplies only runtime
media/presentation, and each App deploys its pair in its own `assets/<app>`
root. A repair must not recreate a YAML/catalog machine selector or
board-specific second execution path.

This candidate follows [chip extraction](m5-shared-chip-extraction.md),
[common PC integration](../history/M5-T540-shared-ibmpc-integration-proposal.md) and
[independent Apps](../history/M5-T543-four-pc-apps-proposal.md). Its ledgers must address the
resulting `core/chips` chip owners, `core/board-base`, `core/board-at` and
`core/board-xt` board mechanisms, `core/machine` composition, and App-specific
Profile inputs, using the admitted deployment map. Migration does not replace
the source/function/timing qualification required here. Historical paths in
supporting evidence remain provenance only.

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

## T555 Execution Order

T555 begins with a current-path audit rather than reusing a historical
controller finding as implementation authority. Its S1 ledger assigns every
retained route to one of three outcomes: a current source-backed route, a
bounded Core/board repair, or an explicit L2/L1/L4 boundary. Later S work is
admitted in this order so a cross-device repair never invents an input for its
neighbour:

1. PIC, PIT, DMA and refresh/IRQ board routes.
2. RTC/CMOS, KBC/XT keyboard/PPI and their reset/NMI/clock routes.
3. FDC/media and the four HDC personalities, including their PIC/DMA/media
   consumers.
4. CGA/EGA/CECG video and profile-selected aperture/clock/presentation routes.
5. Cross-profile board reset, firmware, deadline and integration closure.

The S3 original-source reconciliation assigns the complete DeskPro KBC
speed-control class to S6's cross-device board receiver: A3/A4 selection,
A5 no-OBF special read, auxiliary PIT counter-2 refresh trigger and CPU HOLD
extension with DMA arbitration. The [S3 source ledger](../etc/evidence/t555-s3-rtc-keyboard-source-ledger.md)
records the exact original authorities and rejects mapping speed to output
bits 2/3. Missing A1/A2/A6 definitions require source reconciliation, not
invented firmware behavior. This transfer keeps every member in T555's
completion predicate; a successful boot or retained timing model cannot
close a confirmed missing function or connection.

Each admitted S must use current paths, identify its source tier, preserve the
single owner, and distinguish a genuine repair from a proof-only closure. A
fact outside the selected controller, adapter, drive, monitor or board source
stays explicitly L2/L1/L4; it is not converted into a compatibility constant.

## Exit And Boundaries

One device/port/memory/deadline owner; no BIOS-specific responses, parallel
scheduler, second framebuffer or generic-ATA substitution. Exact-source
values/formulas are L3, model/ratio estimates L2, order-only behavior L1.
Run complete unit and required board/integration regressions and account for
every List 1/List 2 row before claiming a controller complete.
