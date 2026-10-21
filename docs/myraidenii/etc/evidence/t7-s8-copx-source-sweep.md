# M1 T7 S8 — COPX-D2 Source Sweep

## Evidence-grade rule

The owner restated the product policy on 2026-10-20: an L3 fact requires an
inspected primary manual PDF containing the directly applicable formula,
constant or rule.  A range, a board observation or an external implementation
is L2.  A local choice without either source is L1.

## Sources inspected

The following public material was inspected on 2026-10-20 without downloading,
retaining or importing any protected payload or source:

| Source | Direct fact | Grade | Limitation |
| --- | --- | --- | --- |
| [MAME Raiden II driver](https://github.com/mamedev/mame/blob/master/src/mame/seibu/raiden2.cpp) | The selected board declares `COPX-D2` at U0313 as a separate 0x40000-byte region.  Its hardware note calls lookup/math use *probable*. | L2 | The driver gives no finite request, index, result, reset or failure contract for this image. |
| [Arcade Museum ROM identity record](https://www.arcade-museum.com/tech-center/rom-identifier?hash=a6732ff9) | `copx-d2.u0313` is listed as a 262144-byte COPX-region Raiden II image. | L2 | Identity/size does not establish a Board consumer. |
| [31C3 preservation slide](https://fahrplan.events.ccc.de/congress/2014/Fahrplan/system/attachments/2564/original/Preserving_Arcade_Games_-_31c3_LORES.pdf) | A photographed board component is labelled `COPX-D2`, copyright 1992 RISE Corp. | L2 | The slide is not a primary device manual and gives no electrical or software contract. |

No primary COP/COPX, Seibu board, or RISE component manual PDF with a direct
applicable register rule, lookup formula or response constant was found in the
bounded lawful search.  No external emulator implementation was copied,
transliterated, linked, or treated as a hardware fact.

## Current-source cross-check

The code still has one deliberate twelve-role Product-to-Machine image
transaction.  Board COP has no COPX storage, address range, request, callback
or result field.  That is correct: adding the image path alone would create an
inert second input route rather than a modeled device.

## Disposition

COPX-D2 remains the High-priority deferred Board/COP receiver in
[TODO](../../states/TODO.md).  There is still no finite contract for a later
implementation packet: it must establish the request mechanism, index or
address calculation, result width/order, reset/failure behavior and a
synthetic non-ROM oracle.  An external behavior model may be admitted as L2
only after those finite facts are separately documented; a local guessed
lookup would be L1 and is not introduced here.

