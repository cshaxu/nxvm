# Selected-Profile Model-L3 Phase Refinement

## Purpose

Evaluate and implement a source-labelled model-L3 phase representation for one
selected machine profile after the named corpus is complete. This is not a
repository-wide switch and cannot be admitted from deterministic L3 order
alone. Without physical measurement it must not claim physical cycle exactness.

## Required scope

Only after instruction-timed execution, bus-timed PC/AT operation, and the
PC/AT device service-timing corpus have completed, select one profile,
source-labelled CPU timing model, prefetch assumptions, bus phase model, and
device microstates. Establish a single phase/ownership representation that
composes CPU execution, memory/I/O wait, DMA arbitration, interrupt
acknowledgement, PIT/RTC clocks, selected storage readiness, and reset. Explain
every retained difference from actual hardware as a named profile boundary.

## Non-goals and stop conditions

No claim for every historical PC/AT, physical cycle exactness, analog/video/audio
fidelity, x87 timing, host real-time pacing, Windows compatibility, or a copied
third-party emulator. Stop if the selected source/corpus cannot define an
observable model phase contract, or if it demands an unadmitted peripheral owner.

## Evidence standard

Require profile/corpus provenance, phase-to-source-and-model ledger, deterministic
replay trace with instruction/bus/device boundaries, reset/cancellation proof,
selected firmware/guest consumer result, and explicit non-selected profile
transfers. Completion is profile-specific, names every model-derived and
reference-exhausted boundary, and never retroactively upgrades T354 selected
L3 into a physical claim.
