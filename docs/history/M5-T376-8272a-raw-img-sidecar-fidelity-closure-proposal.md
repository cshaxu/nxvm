# 8272A Raw-IMG Sidecar Fidelity Closure

## Purpose

Close the currently deferred 8272A Deleted-Data/Control-Mark fidelity and Scan
command routes without replacing the established raw floppy image exchange
format. A raw `.img` remains the sector-payload file; an optional,
human-readable, same-basename `.img.json` sidecar carries only metadata that a
raw sector dump cannot express. This candidate is a shared FDC repair and is
an explicit prerequisite of the final IBM PC/AT 5170 Model-339 L3 audit.

It follows the already-admitted 5170 board/device phase-timing task. That task
may establish the common FDC availability and phase contract, but does not make
a Deleted/Control-Mark/Scan claim. This candidate must replay and bind those
admitted timing rules for the newly completed command paths before they can be
used as final-audit evidence.

## Required scope

Define and implement one optional sidecar lifecycle for a mounted raw image:

- `fdd.img` remains ordinary raw sector payload and is independently usable by
  existing tools; `fdd.img.json` is not a new image/container format, is never
  required for an ordinary image, and contains no sector payload.
- The versioned JSON schema records the image geometry/identity checks needed
  to reject a stale or mismatched sidecar, and per physical sector position
  (including an unambiguous physical occurrence for duplicate CHRN IDs) the
  Deleted-Data Address Mark attribute. It is deterministic, human-readable,
  validated before use, and rejects malformed, incompatible, or ambiguous
  records rather than silently assigning a mark.
- Mount, write, format, eject/reinsert, read-only, rename/copy/import/export,
  failed save, and sidecar-absent paths have one owned, failure-atomic
  lifecycle. A raw image without a sidecar means every represented sector has
  the ordinary data mark. Media changes update the raw payload and its sidecar
  as one recoverable media-pair operation; no host filesystem shortcut may
  mutate guest-visible state outside the FDC/provider contract.
- Implement the 8272A Read Deleted Data and Write Deleted Data command/result
  paths, and the normal-read versus deleted-read ST2 Control-Mark distinction,
  through the existing FDC/DMA/IRQ/reset route. No result is inferred from
  payload bytes.
- Implement the independently unfinished Scan Equal/Low-or-Equal/High-or-Equal
  command families through their documented transfer, comparison, result, DMA,
  IRQ, reset, cancellation, and selected timing path. Scan is controller
  behavior, not persisted metadata; its interaction with a marked sector uses
  the same explicit mark contract.
- Add focused raw-only and raw-plus-sidecar regressions, including stale or
  malformed JSON rejection, duplicate-ID selection, guest-visible result
  bytes, read-only failure, failed paired persistence, removal/reinsertion,
  and no regression to ordinary raw IMG use. Keep vendor ROMs and guest media
  external to Git.

## Non-goals and stop conditions

No new floppy image/container format, flux/weak-bit/CRC/physical waveform
emulation, automatic metadata inference, hidden test-only injection, generic
host filesystem API, guest-media or firmware commit, or IBM factory-equipment
claim. This does not expand the selected 5170 configuration or turn the
aftermarket 1.44 MB drive into original equipment.

Stop for owner direction if the lifecycle requires a second media format, if
the existing provider cannot guarantee the paired update boundary, or if an
unresolved FDC timing rule needs a change outside the accepted T375 contract.
Transfer any general media-provider defect to its earliest owner; do not hide
it in the 8272A command implementation.

## Evidence standard

Require a schema and lifecycle contract, source-labelled 8272A command/result
matrix, raw-versus-sidecar media-pair matrix, focused command/DMA/IRQ/reset
traces, failure-atomicity proof, and a replay against the completed T375 FDC
timing ledger. The final 5170 audit receives an explicit supported/unsupported
matrix: ordinary raw image behavior, optional sidecar behavior, and every
remaining non-goal are separate rows.
