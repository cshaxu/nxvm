# M6 T36 Snapshot State

## Result

T36 delivered MyNes snapshot state using the SoftPC ownership model. App owns
`save <file>` and `load <file>`, paths and atomic replacement. Common retains
its existing serialized state-operation/lifecycle boundary. Core owns the
versioned `MNS1` image, cartridge identity check, state restore and failure
isolation. ROM bytes and paths never enter the snapshot.

## Evidence

- `c604cb7`: M6 design, schema and UX proposal.
- `e3acddf`: Core image plus Driver state-stream hooks.
- `141d934`: Console commands and atomic save path.
- `6dbaa3c`: Common/Driver/Core save-stop-load transaction probe.
- `e63ba8e`: App command end-to-end probe.
- `0d8805d` and `98a0700`: mismatched-cartridge and truncated-image rejection.

The probes cover save while running/paused, load only while stopped, restored
paused state, content identity mismatch, invalid magic, truncation, missing file
and preservation of the current state on rejection. Full x64 and x86 suites
passed 117/117 each after the snapshot transaction implementation; the final
focused dual-architecture rejection tests also passed.

## Boundary

The image is product-private and versioned. It references the currently inserted
matching cartridge instead of embedding ROM content. Save slots, compression,
rewind, replay, networking and a Common/Lib snapshot ABI remain excluded.