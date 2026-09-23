# IBM 5170 Native Floppy Contract

> Retained proposal for the completed T497 task.

## Objective

Correct the immutable IBM 5170 Model 339 diskette contract: Drive A is the
original 5.25-inch 96-TPI 1.2 MB high-capacity drive, with a supported 360 KB
compatibility-media selection. A Model 339 session must not default to, or
silently accept, later 3.5-inch 720 KB or 1.44 MB media.

## Primary Basis And Boundary

The IBM *PC/AT Technical Reference*, 1502243, March 1984, System Board CMOS
diskette-drive-type table defines type `01h` as a double-sided 48-TPI drive
and type `02h` as a high-capacity 96-TPI drive. Its compatibility text says
the high-capacity drive reads and writes 160/180 KB, 320/360 KB, and 1.2 MB
diskettes. These facts are Manual L3. The task will visually preserve the
printed table/page reference in its source ledger; OCR is only an aid.

The task does not claim mechanical double-step pulse timing, spindle behavior,
or drive-ready physics that the present FDD model does not own. It changes
only the selected immutable machine drive and explicit mounted-media geometry.
It neither imports firmware/media nor infers geometry from image length.

## One-Path Design

The immutable Model 339 descriptor remains the owner of the physical drive
CMOS type (`02h`). A session may make one explicit media request: absent or
`1200k` selects 1.2 MB; `360k` selects compatible 360 KB media; `720k` and
`1440k` are invalid for this profile. The existing session `floppy_kind` then
flows once into the existing FDD geometry constructor. The same typed request
grammar keeps `default-at` as its distinct four-format policy; it does not
make any later 3.5-inch format valid for Model 339. The FDD remains the
sole mutable media-geometry owner and Core remains the sole FDC owner. There
is no image-size probe, profile-side FDC state, alternate controller, or CMOS
rewrite based on mounted media.

## Subtasks

1. Freeze a visually checked original-source ledger and bounded external-model
   corroboration record for the Model 339 drive contract.
2. Freeze the complete List 1 of drive, CMOS and media-selection facts,
   including each explicit non-claim.
3. Freeze the current-code List 2 and its one owner-local repair batch.
4. Implement the one-path descriptor/session selection correction, delete the
   false field-upgrade/1.44 MB assertions, and add focused positive and
   negative composition tests.
5. Run Release BYOB boot replays for selected 1.2 MB and 360 KB media, record
   semantic terminals only, and rerun affected profile/FDC gates. A terminal
   failure transfers its first owning hardware batch; it is not hidden by a
   fallback or a media alias.

## Acceptance

The descriptor reports CMOS type `02h` and no field-upgrade default. Model
339 session construction produces 80x2x15 geometry by default and for an
explicit `1200k` request, accepts explicit `360k` as 40x2x9 compatibility
media, rejects `720k` and `1440k`, and leaves Default-AT's four-format policy
unchanged. Focused composition tests and selected Release replays pass without
committing protected media or local asset paths.
