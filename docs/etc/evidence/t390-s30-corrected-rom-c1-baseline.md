# T390 S30: Corrected-ROM C1 Baseline

## Outcome

The project-owned synthetic capture still proves prefix-aware aggregation and the
C0-to-C1 diagnostic transition. The corrected-ROM formal-asset replay was then
contained to two million retirement attempts and a forty-five second process
limit. It completed within the wall-clock bound with two million classified
retirements, zero source-unallocated observations, one aggregate form, no
protected-mode observation, and no protected-return checkpoint.

Therefore C1 was not reached. This is not evidence that C1 is empty, timed, or
qualified. It establishes that the current corrected-ROM Model-40 start path
does not provide a meaningful C0-to-C1 CPU baseline.

## Capture correction

The test-only capture summary now reports normalized protected-mode and
checkpoint state alongside its existing aggregate count and form count. This
makes the containment result distinguish a real C0 boundary from a repeated
pre-protected start path without retaining any asset identity, locator, digest,
byte, PC, trace, or provenance.

## Transfer

The first receiver is the existing DeskPro board-level startup/behavior path:
it must establish a bounded meaningful POST or early protected-mode checkpoint
under the accepted ROM decode before CPU C1 capture is reattempted. The physical
retirement guard remains unchanged; no timing value, board clock, firmware boot
claim, or L3 conclusion follows from this result.
## Read-only bridge assessment

PCjs independently declares the same four corrected Rev-E ROM windows and a
Compaq-specific 128 KiB relocation/write-protection behavior.  86Box provides
a useful second implementation of that relocation class, but its locally
available DeskPro ROM choices are later revisions and its memory configuration
is not the selected 1 MiB Model-40 contract.  These references therefore
confirm the direction for investigation; they do not authorize an imported
implementation or a changed NXVM board contract.

A dynamic cross-emulator bridge was not run: the local PCjs checkout has no
installed runtime dependencies and no local 86Box executable is available.
No external code, ROM, media, trace, checkpoint value, or generated output was
retained.  The bounded NXVM replay remains the only executed aggregate
baseline for this S.

## Full-gate hygiene

Two pre-existing test-only API drifts were uncovered by the required clean
full build and repaired: the platform request bridge smoke now uses the
existing key-event request contract, and the CPU-probe test support now uses
the existing internal session declaration and pointer-form control calls.
Neither change alters a Core or VM production interface, data layout, or
runtime behavior.  The full build and 284-test current gate pass after these
repairs.