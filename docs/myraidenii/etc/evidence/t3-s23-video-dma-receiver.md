# T3 S23: Video-DMA Receiver Contract

## Scope and Evidence Grade

This is an evidence-only S.  It consumes receiver 3 in the accepted T3 S20
residual contract, after S22 completed the common banked setup and safe-point
owner.  No production, test, ABI, manifest, asset, ROM or executable changes
are made here.

The fixed external reference remains the read-only MAME revision
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`, with the exact four source hashes
recorded in [references](../research/references.md) and S20.  Its statements
are source-grade L2 research only: no external implementation, table or ROM
content was imported.  Current local code is inspected directly at S22
`52fc45e98`.

## Current Local Boundary

`myraidenii_core_cop` has the sole pending-operation owner and its checked
Core-memory helper.  `47e` selects one of 512 contexts; `478`, `47a` and
`47c` are byte-merged source/size/destination words in that selected context.
`476` remains a distinct scalar configuration word.  Trigger `6fc` is admitted
only when the selected raw mode has an implementation.  `45a`--`45d` are
explicitly unmodelled and reject writes.  A successful trigger is serviced
only through `myraidenii_core_cop_advance()` at Board's existing safe point.

`myraidenii_core_video` currently owns only its L2 raster phase.  Board owns
the Video object, resets it with the COP, and converts its VBlank edge into the
Board's one interrupt latch.  There is no tilemap, palette, brightness latch,
frame or copied DMA path today.  Therefore accepting modes `14`, `15`, or
`80`--`87` now would be a false success; copying them to ordinary guest RAM
would also violate the identified private-output ownership.

## Complete Mode Disposition

| Mode | Source-grade fact retained from S20 | Correct local owner | Admission disposition |
| --- | --- | --- | --- |
| `14` | The reference has a private tilemap transfer of `0x2800` bytes.  Its apparent hard-coded source/destination relationship is not a general descriptor and may reflect odd setup fields. | Video owns a 0x1400-word accepted tilemap latch; COP owns checked source reads; Board composes them. | Blocked pending a bounded, selected-set descriptor trace or equally specific non-ROM hardware evidence.  Do not encode the reference address. |
| `15` | The reference has a private 2048-word palette transfer of `0x1000` bytes, while the significance of its request length is not established. | Video owns the accepted palette words and reset; a later renderer converts them. | Blocked pending a fixed-size/descriptor eligibility decision and RGB-word interpretation evidence.  No COP palette mirror. |
| `80`--`83` | Source-only brightness family; a general transfer observation alone does not establish selected-set use or colour semantics. | Video, if later qualified. | Blocked: exact affected latch, channels, flag and rounding behavior are not established. |
| `84` | Source-only mode; no local receiver or selected-set oracle. | Video, if later qualified. | Blocked. |
| `85` | Source commentary indicates an RGB555 blend candidate with a source bit-15 exemption, but that is not an independent selected-set or silicon oracle. | Video, if later qualified. | Blocked: no copied blend expression or inferred rounding. |
| `86`--`87` | Source-only modes; no independent channel or destination evidence. | Video, if later qualified. | Blocked. |

The eight brightness modes remain one family for coverage purposes: no member
may be admitted merely because another mode passes bytes through.  The source
observation that some nonselected games use these paths is negative evidence
against treating them as a Raiden II contract.

## Later Receiver Shape

When descriptor evidence exists, the smallest correct implementation is still
local to MyRaidenII Core:

1. Board constructs one private Video receiver when constructing COP.  It does
   not expose a Lib, Emulator or public Machine capability.
2. COP snapshots the selected context into its existing one pending operation,
   reads guest bytes through its existing checked Core-memory owner, and sends
   complete accepted bytes to that receiver only at its existing safe point.
3. Video validates destination/length, commits one complete private latch, and
   leaves its prior accepted latch unchanged on any failure.  Board propagates
   the existing `lib_status`; it adds no second DMA queue, status mirror or
   retry path.
4. Reset clears the private Video latches through the current Board reset
   order.  Rendering later consumes those exact accepted latches; it does not
   create a second guest-memory transfer.

This is deliberately not implemented yet.  The missing descriptor and colour
facts determine fields and failure order; choosing them now would turn a
research placeholder into an undocumented hardware model.

## Required Evidence Before Code Admission

- A finite selected-set execution observation or equally specific original
  board documentation for mode `14` that distinguishes actual configured
  source, destination and length from the reference's fixed address path.
- The corresponding mode `15` transfer eligibility plus the palette word
  format and reset/publication meaning.
- For every brightness mode proposed for admission: selected-set applicability,
  input/output latch identity, channel arithmetic, bit-15 behavior and rounding
  oracle.  A mode without these facts remains explicitly rejected.
- A focused Board test plan using the existing Board executable: successful
  complete transfer, source failure preserving the old Video latch, invalid
  descriptor refusal, pending/reset behavior and actual guest MMIO admission.

No screenshot, static source comment, attract-screen reachability or generic
RAM copy substitutes for these requirements.

## Closure Review

The full S20 receiver-3 universe was reviewed: `14`, `15`, `80`--`87`, their
common context/register lanes and trigger `6fc`.  All are visible and none is
silently accepted.  The current production/test surface is intentionally
unchanged; S22's existing dual-width qualification remains historical evidence,
not a new runtime claim.  T3 remains open for the four residual macros and
finite selected-set startup, and the separately queued Video task receives the
blocked rendering/DMA work.
