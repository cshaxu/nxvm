# M5 T465 Core Intel 8272A Logical Media Phase Contract

## Active Record

T465 owns the queued Intel 8272A/controller-media phase contract. Its source and
code-ledger reconciliation must complete before implementation is admitted.

## S1 Accepted Evidence

`6d2a96a5` reconciled the original fifteen FDC rows, but its uPD765-led source
wording was superseded by T465 S4. Intel's original 8272A material is the sole
selected chip authority; IBM remains the AT board authority; NEC and the five
emulators are comparison only. The renamed S13/S14 ledgers re-derive every row
as Manual L3, Other/board L3 or fallback to L2 and retain all gaps in the one
Core FDC/media path. No media grammar, drive mechanics, board clock or second
owner is admitted by that correction.

## S2 Accepted Evidence

`0ce5ef6f` corrects the one direct reset defect in the sole `fdc.c` owner.
The existing reset helper now saves and restores only SRT/HUT/HLT, the three
Specify fields the rendered NEC reset record requires it to preserve; NDMA and
all cancellation behavior retain their pre-existing reset path. The focused
smoke passes and prints `M5:T465:S2:FDC-reset:OK` after both DOR reset edges
and reset-Sense drain. Production code grows by six lines; no ABI, profile,
scheduler, controller, media path or compatibility branch was added.

## S3 Accepted Evidence

`bcbc3040` corrects the ledger rather than adding a second FDC model: Core's
selected Intel 8272A-compatible identity makes `10h` the existing invalid ST0
`80h` command, while NEC uPD765 Version/overrun differences remain conditional
Other/board L3 or fallback to L2. The focused smoke proves `10h` returns
`80h`, raises no IRQ and returns to command phase, then prints
`M5:T465:S3:FDC-8272-command:OK`. Production code is unchanged; the only test
addition publishes this existing proof. No model selector or media/timing path
was introduced.

## S4 Accepted Evidence

`c43958b3` corrects the active task's chip authority and rebuilds both finite
ledgers around Intel 8272A.  Rendered Intel pages, IBM AT facts and each of
86Box, MAME, PCjs, Bochs and QEMU are recorded with their actual disagreement:
family extension, limited subset, or later-controller behavior cannot extend
the selected command set.  The focused FDC smoke and documentation governance
pass.  The audit leaves two Manual-L3 implementation gaps explicit--parallel
Seek state and a board-validated clock conversion--and leaves unselected
physical/media/firmware terms at L2.  No source, ABI, controller path, parser,
scheduler, media owner or artifact changed.
