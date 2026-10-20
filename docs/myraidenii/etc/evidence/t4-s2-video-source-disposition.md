# T4 S2: Video/Sprite Source Disposition

## Question

T4 S1 requires a lawful consumer contract before `0x6ce--0x6cf` can receive a
Video/Sprite implementation.  This S audits the evidence sources presently
available to the repository and the public, rather than deriving state from
emulator source or protected game data.

## Reviewed Sources

| Source | Grade | What it establishes | Why it cannot admit the receiver |
| --- | --- | --- | --- |
| Owner-supplied external `raiden2.zip` metadata | Owner asset identity | The existing metadata inventory confirms 14 archive entries and 13 runtime image identities, including four sprite-role images. | Filenames, lengths and hashes contain no register width, field, reset, consumer, decryption or failure contract.  The archive was not extracted or inspected as content. |
| Original RAIDEN II operator manual, already pinned in the research register | Original publication | Cabinet orientation, controls, DIP/operator and connector context. | Its three pages contain no PCB schematic, custom-chip programming material, register map, graphics layout or sprite state behavior. |
| Publicly indexed board repair material | Secondary commentary | A physical PCB exists and uses custom devices; some repair material itself notes that it is not an electrical schematic. | It is neither an original register specification nor a reproducible behavioral oracle.  It cannot define a model. |
| Pinned MAME RAIDEN II driver | L2 research | A named map direction/owner hint and board-layout observations. | The unit is LGPL reference code under the source policy.  Its handlers and inferred algorithms are not imported, transliterated or used as the missing semantic specification. |
| Public searches for a Seibu service manual, board schematic, custom-chip data sheet and sprite-protection register documentation | Discovery only | No original source establishing the selected flag-family contract was located. | Search absence is not proof that no source exists.  It is only a disposition of the sources presently available to this task. |

## Required-Contract Check

| Receiver prerequisite from T4 S1 | Present lawful evidence | Result |
| --- | --- | --- |
| Write width and byte order | Map range only. | Missing. |
| Named fields/bit meanings | None. | Missing. |
| Reset and readback contract | None. | Missing. |
| Consumer and guest-time latch point | Generic Video/Sprite ownership only. | Missing. |
| Relationship to decryption/frame state | Dedicated board assets/devices are identified, but no model. | Missing. |
| Finite ROM-free success and failure oracle | None. | Missing. |

No column is complete, so no production or test receiver is eligible.  In
particular, a two-byte stored latch would make the startup progress farther but
would falsely claim a state model without knowing when or whether any consumer
uses it.

## Disposition And Next Admission Condition

The Video/Sprite receiver remains blocked by source evidence, not by a Shared
or host-output interface.  The next implementation S may be admitted only
when the owner supplies or authorizes a source with a finite contract covering
all six prerequisites above, or when a distinct independently documented
Video mechanism becomes available and does not rely on this unknown family.

The blocked state does not authorize using raw ROM bytes, disassembly,
instruction/register traces, screenshots, copied MAME handlers, generic RAM
or a no-op acceptance path.  It also does not close T4, T3 or the playable
MyRaidenII goal.
