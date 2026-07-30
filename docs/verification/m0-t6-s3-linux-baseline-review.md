# M0 T6 S3 Linux Baseline Review

## Decision

The whole-NXVM M1 baseline preserves its existing Linux platform source instead
of deleting it during the Windows-focused import. This keeps a future Linux DOS
runtime path open without changing the present Windows delivery plan.

## Boundary

M1 acceptance remains one Windows GCC build and run. A future Linux target may
reuse the final `machine + dos` core through a Linux platform adapter; it must
define its own build, test, asset, and release contract before making a
compatibility claim.
