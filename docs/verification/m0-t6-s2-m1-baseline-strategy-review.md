# M0 T6 S2 M1 Baseline Strategy Review

## Decision

M1 establishes a GCC-runnable whole-NXVM baseline before subtractive
refactoring. This avoids guessing the minimum set across legacy global state,
BIOS, device, and platform couplings.

## Governance Result

The temporary `src/nxvm-baseline/` area is limited to baseline reproduction;
the final module boundaries remain authoritative. The two owner-provided local
disk-image fixtures are identified by logical name, size, and SHA-256 only.
They remain outside Git and releases. M1 has not started.
