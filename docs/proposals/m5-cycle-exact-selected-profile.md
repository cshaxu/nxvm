# Cycle-Exact Selected Profile

## Purpose

Evaluate and, only with a primary hardware contract plus a named corpus,
implement cycle-exact behavior for one selected machine profile.  This is not
a repository-wide switch and cannot be admitted from deterministic L3 order
alone.

## Required scope

After instruction-timed execution and bus-timed PC/AT operation, select one
profile, clock source, CPU/prefetch behavior, bus phase model and device
microstates.  Establish a single phase/ownership representation that composes
CPU execution, memory/I/O wait, DMA arbitration, interrupt acknowledgement,
PIT/RTC clocks, selected storage readiness and reset.  Explain every retained
difference from actual hardware as a named profile boundary.

## Non-goals and stop conditions

No claim for every historical PC/AT, analog/video/audio fidelity, x87 timing,
host real-time pacing, Windows compatibility, or a copied third-party
emulator.  Stop if the selected primary manual/corpus cannot define an
observable phase contract, or if it demands an unadmitted peripheral owner.

## Evidence standard

Require profile/corpus provenance, phase-to-source ledger, deterministic
replay trace with instruction/bus/device boundaries, reset/cancellation proof,
selected firmware/guest consumer result, and explicit non-selected profile
transfers.  Completion is profile-specific and never retroactively upgrades
T354 selected L3.
