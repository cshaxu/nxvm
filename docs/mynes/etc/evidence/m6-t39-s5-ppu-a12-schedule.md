# M6 T39 S5 PPU A12 Schedule Diagnosis

## Finite Observation

An ignored build probe starts the owner-local Mapper-004 input until it has
enabled rendering, then advances one PPU frame directly from that machine
state.  For this state, the PPU control selects background patterns from
`$0000` and sprite patterns from `$1000`.  The probe resets only the
Cartridge's diagnostic counter state and counts counter transitions produced
by the represented PPU fetches.  It observes zero qualified A12 rises over the
frame.

This is not an owner-local media assertion.  It follows from the project
source: `core_ppu_fetch_sprite_pattern` returns immediately for every empty
sprite slot.  Real PPU rendering still performs dummy sprite-pattern fetches
for unused slots.  With the selected tables, those dummy fetches must present
A12 high after the background's sufficiently long low period, yielding the
MMC3 scanline-clock edge.

## Disposition

| Candidate | Result |
| --- | --- |
| Cartridge `$C001` reload contract | Already repaired by S4; not reopened. |
| CPU controller and IRQ delivery | Not selected by this PPU-only edge-count experiment. |
| PPU empty-sprite fetch ownership | Selected: returning before the required dummy fetch suppresses the represented A12 rise. |

S6 must preserve the zero pixel data of unused slots while reading the
appropriate dummy sprite-pattern address through the same PPU memory path.
Its owned regression must demonstrate one qualified Mapper-004 edge from a
rendered scanline with no selected sprites and background `$0000`/sprites
`$1000`.  It must then re-run the ignored Start/no-input frame comparison.

Reference: [NESdev MMC3 IRQ specifics](https://www.nesdev.org/wiki/MMC3),
including the documented dummy tile `$FF` fetches for unused sprite slots.
