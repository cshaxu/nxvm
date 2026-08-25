# M5 T465 Core uPD765 Logical Media Phase Contract

## Active Record

T465 owns the queued uPD765/controller-media phase contract. Its source and
code-ledger reconciliation must complete before implementation is admitted.

## S1 Accepted Evidence

`6d2a96a5` reconciles all fifteen FDC source and code-gap rows. Rendered NEC
and IBM material remains normative; 86Box, MAME, PCjs, Bochs and QEMU only
corroborate labelled model observations, and QEMU's 82078 model is excluded as
a uPD765/AT authority. Each row now names Manual L3, Other/board L3 or
fallback to L2, with all gaps retained by the existing Core FDC/media path.
The next batch may implement only the finite Manual-L3 command/revision cases;
no media grammar, drive mechanics, board clock or second owner is admitted.

## S2 Accepted Evidence

`0ce5ef6f` corrects the one direct reset defect in the sole `fdc.c` owner.
The existing reset helper now saves and restores only SRT/HUT/HLT, the three
Specify fields the rendered NEC reset record requires it to preserve; NDMA and
all cancellation behavior retain their pre-existing reset path. The focused
smoke passes and prints `M5:T465:S2:FDC-reset:OK` after both DOR reset edges
and reset-Sense drain. Production code grows by six lines; no ABI, profile,
scheduler, controller, media path or compatibility branch was added.
