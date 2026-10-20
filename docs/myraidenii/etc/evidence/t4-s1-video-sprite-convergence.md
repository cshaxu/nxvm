# T4 S1: Video/Sprite Convergence Ledger

## Scope And Method

This record consumes the first planning batch of the queued Video task.  It
does not modify production or test code, a public interface, media, ROM
inventory, or an executable.

The review uses the local Board/Video implementation and the product research
register.  The fixed MAME RAIDEN II driver remains an LGPL research reference
only.  It supplies L2 address/direction observations; its handlers, algorithms
and tables are neither copied nor treated as a complete hardware
specification.  The original RAIDEN II operator manual identifies the cabinet
and controls, but contains no board register, sprite or graphics data-layout
contract.  No protected guest data, trace, register value, program counter or
screenshot is retained by this review.

## Present Owner Boundary

`myraidenii_core_board` owns the bus map, the sole VBlank-derived interrupt
latch, reset order and the configured V30 Machine attachment.
`myraidenii_core_video` owns only an accepted private L2 raster phase:
8 MHz pixels, 512 pixels per line, 282 lines per frame, and a VBlank edge
after 240 lines.  It does not own a register file, tilemap, palette, sprite
state, graphics decoder, frame or output contract.  COP owns its own pending
operation and checked Core-memory transport; it is not a Video state mirror.

The accepted Shared RGB32 frame contract can carry a future complete frame.
That is a host receipt, not evidence for a RAIDEN II palette, rotation, decode
or composition mechanism.  KVM Console remains text-only.

## Finite Video/Sprite Ledger

| Domain | Established fact and grade | Correct owner | Disposition and missing admission facts |
| --- | --- | --- | --- |
| Raster/VBlank | Existing local L2 model is routed through the real Board attachment and one interrupt latch. | Core Video, observed by Board. | Accepted only as its stated L2 cadence.  It is not a frame, scanline, priority or sprite-timing model. |
| `0x6ce--0x6cf` flags write | L2 reference map names a write-only sprite-protection flags receiver; selected-set execution next faults there. | Core Video/Sprite. | Blocked.  Required: stored width/byte order, bit meanings, reset value, readback/failure behavior, relation to decryption or rendering, and a lawful oracle.  Do not admit a no-op, ordinary RAM, or Board/COP mirror. |
| Adjacent sprite-control routes | The same L2 map distinguishes `0x6c0--0x6c7` and `0x6d8--0x6df` from the flags route; local Board already retains distinct existing owners/routes where qualified. | Core Video/Sprite, except existing Board bank control. | Blocked as one register-family investigation.  Address labels alone do not establish semantics, commit order or data flow. |
| Sprite buffer/storage | The L2 map identifies a separate sprite-buffer region, but no independently qualified element format, buffering phase or ownership transition is available. | Core Video/Sprite. | Blocked.  Required: finite entry layout, capacity, byte order, latch point and reset contract.  Do not expose it as generic guest RAM. |
| Tile/background/text data | Research identifies character/background assets and a CRTC-related register block, but not selected-board tile word layout, layer count, scroll register semantics or character decode. | Core Video. | Blocked.  Required: original board data or lawful finite evidence for each layout and register family. |
| Palette and brightness | Earlier T3 ledger identifies a candidate 2048-word palette transfer and brightness family only from the L2 reference. | Core Video. | Blocked.  Required: palette word format, transfer descriptor/eligibility, reset and channel arithmetic/rounding.  RGB32 transport does not supply these facts. |
| Sprite decode/decryption | Board research identifies dedicated graphics/protection devices, but no independent key, transform or selected-board decoder contract. | Core Video/Sprite. | Blocked.  A reference implementation or ROM-derived table cannot fill this gap. |
| Priority, scroll and composition | No independent layer order, priority, scroll or clipping contract has been identified. | Core Video. | Blocked.  Do not create a renderer that guesses ordering from a title-screen expectation. |
| Complete frame publication | Shared RGB32 supports complete host frames; MyRaidenII has no Machine adapter or Video-complete frame. | Future Machine adapts a complete Core Video frame. | Deferred after all source state above is qualified.  One copied completed frame, no partial/stale publication on conversion failure. |
| Portrait rotation and host display | The L2 reference declares native 320x240 with ROT270; physical aspect and host scaling are distinct. | Future Machine output conversion. | Deferred after native frame generation.  Do not mutate guest RAM or make Lib product-specific. |

## Dependency Order And Initial Batch Decision

The immediate startup fault is a useful boundary, not evidence that a two-byte
flags latch is independently implementable.  A receiver cannot be made
coherent before the flag family has an observable consumer contract.  The
smallest lawful next implementation batch is therefore **none**: no production
or test receiver is admitted by this review.

The next Video packet must first supply one bounded, legal evidence source for
the whole sprite-control family.  It must answer, before code is proposed:

1. What the `0x6ce--0x6cf` write represents (width, byte order, fields and
   reset state).
2. Which consumer observes it, and at which guest-time boundary.
3. How it affects sprite data/decryption/frame state, including invalid or
   incomplete writes.
4. A finite ROM-free regression oracle that uses the actual Board route and
   preserves an earlier accepted Video state on failure.

Only then may one owner-local Video receiver be proposed.  It may depend on
the existing Board byte-splitting route and Video reset order, but may not add
a second renderer, event loop, DMA queue, generic RAM fallback, Board/COP
state mirror or Shared change.  Frame publication is deliberately later: it
depends on sprite and tile/palette ownership, not merely flags admission.

## Similar-Issue Sweep

All Video-adjacent local routes were checked against the ledger: the raster
phase has one owner; Board has only the IRQ latch and map routing; COP has no
Video state; RGB32 is one existing host receipt; and no MyRaidenII frame path
or alternate renderer exists.  Sound and Product remain separate queued
domains.  No duplicate owner or reversed dependency is introduced.

## Closure

T4 S1 establishes the complete initial Video/Sprite disposition and rejects
every tempting shortcut.  It does not claim video implementation, a later
selected-set checkpoint, desktop/RDP verification, sound, Product integration
or playable-game progress.  T3 remains open with its own COP obligations;
this record merely identifies its current startup dependency as an explicit
Video evidence blocker.
