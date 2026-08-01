# M5 T12 S9 Baseline Root Removal Provenance

`src/nxvm-baseline` was an M5 migration/reference root. After every retained
NXVM implementation moved under `core`, `vm`, or `vdm`, it contained only
temporary forwarding headers. The direct consumers were changed to canonical
paths before the forwarding tree was deleted.

The root MIT-authorized NXVM source provenance remains recorded by the per-slice
M1/M5 records. This deletion imports no code and removes no user-visible
implementation.

Verification is recorded in `docs/verification/m5-t12-s9-baseline-root-removal.md`.
