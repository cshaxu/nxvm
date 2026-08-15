# T372 S6: Reference-Runtime Non-admission

## Decision

The owner-authorized, revision-pinned external reference experiment is not an
admissible Model-339 observation.  Its configuration controls selected the
candidate IBM AT machine, 80286 at 8 MHz, 512 KB, CGA, no fixed disk, the
selected Rev.3 firmware class, and a removable-drive configuration.  However,
the runtime produced no usable POST, diagnostics, or guest-visible output for
either the candidate removable medium or the AT diagnostics medium.

The attempted configuration therefore does not prove the complete selected
tuple end-to-end.  The owner directed that this reference runtime no longer be
used.  It is retained only as an external, non-product research asset; it
cannot supply a board fact, timing scalar, trace, baseline result, or L3
conclusion.

## Containment And Disposition

No raw trace was retained.  No ROM, guest medium, local pathname, filename,
hash, byte content, asset catalogue, emulator source, executable, or
configuration artifact entered the repository.  The external reference
runtime is not a default dependency and no project source, ABI, CMake, test,
or product profile changed.

The source-backed Model-339 facts in S4 remain the sole admissible factual
baseline.  The unresolved board-phase conversions remain with their existing
Queue receivers.  A future reference experiment needs a separately admitted,
independently observable execution harness; it is optional corroboration and
never a substitute for primary documentation or project-owned probes.

## Similar-Issue Sweep

S6 swept the complete selected tuple, firmware/media containment, fixed-disk
absence, trace retention, reference-tool applicability, and every S1 physical
residual.  The result is non-admission, so no physical value or model-L3
claim changes.
