# M1 T4 S39 — Original Graphics-Contract Source Search

## Scope

S39 searched for provenance-qualified original/vendor Raiden II board material
that could specify a finite graphics element/layout contract. It did not obtain,
download or retain a manual, ROM, screenshot, source file or other protected
payload. It did not inspect MAME/FPGA implementation source.

## Candidate Disposition

| Candidate | Provenance/useful scope | Disposition |
| --- | --- | --- |
| [Raiden II operation-manual index](https://nozomi.nl/downloads/raiden-ii-operation-manual-seibu-kaihatsu.4191/) | The public index identifies a Seibu Kaihatsu Raiden II *operation manual*. It supplies no verifiable graphics-chip, element-layout or attribute contract in its indexed metadata. | Not an implementation authority. |
| [Arcade Museum record](https://www.arcade-museum.com/Videogame/raiden-ii) | Catalog record identifies a three-page operator manual. The record does not expose board/video-layout material. | Not an implementation authority. |
| [Raiden II repair record](https://www.arcade-projects.com/threads/repair-log-raiden-ii-repair-1.33877/) | A third-party repair discussion states that it had no schematic for this board. It is not original/vendor material and cannot prove global non-existence. | Negative search context only; not a graphics contract. |

## Result

No source found in this bounded search provides original/vendor facts for the
tile/sprite element dimensions, bitplane ordering, tilemap word fields,
palette fields, priority, scroll or decryption required by an initial decoder.
The operation-manual records may be useful later for controls/operator UX, but
they do not establish graphics behavior from their available metadata.

Together with S38, this leaves no lawful finite decoder contract under the
current evidence. This is not a claim that no physical documentation exists;
it records only that the reviewed accessible sources do not qualify it. A later
decoder S requires an owner-approved source with those actual facts or a newly
approved behavioral method that can isolate a stable synthetic element without
retaining protected payload.
