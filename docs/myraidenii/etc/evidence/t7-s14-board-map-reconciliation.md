# M1 T7 S14 — Raiden II Board-Map Reconciliation

## Scope and grade

This is a static reconciliation of the local Raiden II Board with the
inspected current MAME Raiden II address map. It uses that map only as an
**L2 address-ownership reference**. It does not reproduce its source, tables,
device behavior or hardware rationale. No inspected primary manual PDF supplies
an applicable Board formula, constant or register rule, so this record
establishes no L3 fact.

The local source reviewed was `src/myraidenii/core/board.c` and its
`board_bus_smoke.c` owner tests. The external map inspected was
`O:/repos.external/mame-raiden2-l2/raiden2.cpp`, `xsedae_cop_mem`,
`raiden2_cop_mem` and `raiden2_mem`, at the pinned research checkout. The
reference is not imported or made a runtime dependency.

## Reconciliation

| L2 address class | Local owner and disposition |
| --- | --- |
| `00000--003ff` RAM; `00800--1ffff` main, sprite, tile and palette RAM | The neutral Machine ordinary-RAM route owns these bytes. The local Board only replaces the selected device windows; `board_bus_smoke` covers ordinary RAM and the `1f000` palette span. |
| `20000--3ffff` bank window; `40000--fffff` immutable program | Board bank callbacks own the window and `0x6cb` is the single-byte bank selector. Immutable program mapping owns the static region. Both lanes and the selector are tested. |
| COP command/register/result ranges in `041c--05b5`, including `0476--047f`, `04a0--04cd`, `0500--0505`, `0580--05b5` | Existing `myraidenii_core_cop_install()` is the one owner. The Board does not create a duplicate decoder; its tests exercise command, result, DMA and sort-trigger routes. |
| Tile-bank-2 `0470--0471` | Board owns a read/write two-byte latch. The map only supports address ownership, so its rendering meaning remains a separate Video concern. |
| CRTC `0600--063f` | Board forwards the existing CRTC window to Video; CRTC register behavior and raster publication retain their existing L2 evidence. |
| Sprite-protection / object window `0640--06bf`, latches `06c0--06df`, output `0762--0763` | Board's private sprite-buffer, upload and latch routes cover the selected windows. Existing deliberate write-only/no-op lanes retain their recorded L2 dispositions, not invented readback behavior. |
| Bank/tile controls `06cb`, `06cc` | `06cb` selects the existing program bank; `06cc` updates tile-bank-01. `06cd` is explicitly retained as the separately accepted no-op byte. |
| Sprite latch `068e--068f` and COP DMA/sort triggers `06fc--06ff` | Board owns the sprite latch; existing COP install owns both trigger pairs. The Board tests use these real paths rather than a shadow control model. |
| Sound `0700--071f`; DIP/player/system inputs `0740`, `0744`, `074c` | Existing Board Sound and active-low input routes own all three finite classes. |

## Differences and disposition

No unowned finite address receiver was found.

Two known differences are not a reason to change a route:

1. The external map identifies `06ce--06cf` as a sprite-flags write endpoint.
   The local Board has the same latch, but neither source provides a finite
   non-table selection rule for blend/shadow rendering. That remains the
   Video-owned TODO receiver at L2; treating the address as a blend algorithm
   would be unsupported L1 behavior.
2. The selected set contains the separate COPX-D2 image. The external map
   identifies the region but gives no request/index/result transaction. The
   local Board deliberately has no path-only COPX attachment. That remains
   the high-priority Core COP/Board TODO receiver.

The reference's non-Raiden-II variants and comments about alternate maps were
excluded; they are not facts about this fixed selected board.

## Result

S14 makes no code, ABI, test, asset or artifact change. It closes the
address-ownership sweep with no finite implementation transfer. It does not
qualify external-ROM startup, gameplay, blending, COPX behavior or physical
hardware timing.
