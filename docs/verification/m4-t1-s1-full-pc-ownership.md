# M4 T1 S1 Full-PC Ownership Verification

- Reviewed `vmachineInit`, `vmachineReset`, `vmachineRefresh`, and
  `vmachineFinal` in the retained baseline to inventory the full-PC path.
- Reviewed `vbiosAddPost` and `vbiosAddInt` registrations to distinguish
  firmware construction from generic device operation.
- `docs/history/m0-m4/firmware-nxvm.md` assigns every M2-listed deferred unit
  to a target owner, migration order, and M1 regression owner.
- No source, guest media, firmware behavior, DOS ABI, platform API, or product
  CLI changed.
