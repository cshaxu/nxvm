# T4 S5: Selected Control Hole L2 Contract

## Admission

The owner authorized bounded L2 behavioral models where no L3 material is
available.  External emulators remain research references only: their code,
algorithms, tables and protected inputs were not copied, translated, linked or
redistributed.

## Bounded Model

The selected RAIDEN II program reaches a terminal fault at `0x696` after the
completed sprite-flags receiver.  The current MAME RAIDEN II memory map does
not assign `0x696--0x697` a device receiver.  The local Board model therefore
independently admits exactly this two-byte span as a write-only, stateless
control hole:

- byte writes at `0x696` and `0x697`, and the Core-split V30 word write at
  `0x696`, are accepted;
- reads and inspection are rejected;
- accesses crossing the span and all neighbouring control bytes remain
  rejected by the existing Board fallback;
- no Video, Sprite, COP, RAM, palette, decryption or frame state is added.

This is an L2 compatibility boundary, not a claim that the physical board had
no device at this address.  It deliberately shares the existing selected-hole
owner rather than creating another register family or generic RAM path.

## Verification

The existing Board smoke directly covers the exact byte/word boundary, no
readback, neighbouring rejection and an actual V30 `MOV moffs, AX` write.  It
passed on x64 and x86.

The bounded owner-ROM probe then ran with its existing transient staging and
cleanup contract.  Both widths advanced exactly one further instruction and
now stop at the same next unmodelled address:

| Host width | Executed | Ticks | Next fault code |
| --- | ---: | ---: | ---: |
| x64 | 68,246 | 1,076,606 | `0x69c` |
| x86 | 68,246 | 1,076,606 | `0x69c` |

The probe retains no program payload or raw trace.  The later `0x69c` stop is
not absorbed by this S; it remains a separately admitted receiver candidate.

After the production change, the complete repository-only unit selection
passed with zero failures on both host widths: `642/642` on x64 and `642/642`
on x86.  Documentation governance and `git diff --check` also passed.  The
earlier full repository run (`669/669` per width) remains evidence for the
unchanged integration paths; this S does not relabel it as a post-change game
integration qualification.
