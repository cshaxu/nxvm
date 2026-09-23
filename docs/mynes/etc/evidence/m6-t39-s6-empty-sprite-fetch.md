# M6 T39 S6 Empty-Sprite Fetch And A12 Filter

## Repair

Unused sprite slots now perform a dummy `$1FE0/$1FE8` pattern read through the
existing PPU memory path while retaining zero sprite pattern bytes.  Sprite
pattern reads run in the PPU's pattern-fetch slots.  Cartridge A12 filtering
now restores raw A12-high state after a short, unqualified low interval
without clocking the counter; only a sufficiently long continuous low interval
permits a qualified rising edge.

Together these changes represent the same PPU bus fact at both owners: the
dummy fetch reaches Cartridge, and adjacent high pattern accesses do not turn
into separate scanline clocks.

## Owned Regression

`mynes.core.mapper4-smoke` creates a rendered scanline with empty OAM,
background patterns at `$0000` and sprite patterns at `$1000`.  It requires
one qualified counter decrement after 341 PPU dots and verifies every unused
slot still stores transparent pattern bytes.  The former implementation either
produced no edge (no dummy read) or would expose repeated edges (short-low raw
line loss).

## Verification And Disposition

| Check | Result |
| --- | --- |
| Ignored PPU schedule probe | 241 qualified rises: exactly one on every visible scanline in the observed background-low/sprite-high layout. |
| x64 focused Mapper-004 linkage | Passed. |
| x64 full MyNES CTest | 53/53 passed. |
| x86 focused Mapper-004 linkage | Passed from a fresh ignored x86 configuration. |
| Ignored synchronized Start/no-input frames | Still identical and black after the bounded scenario. |

S6 restores the missing Mapper-004 scanline clock but does not prove the owner
input has reached an interactive visible state.  The next receiver is the
remaining PPU pixel/scroll pipeline or guest code path after the now-correct
scanline timing; it must be diagnosed separately.

Reference: [NESdev MMC3 IRQ specifics](https://www.nesdev.org/wiki/MMC3).
