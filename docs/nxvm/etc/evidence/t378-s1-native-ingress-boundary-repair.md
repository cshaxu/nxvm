# T378 S1: Native Ingress And Dependency Boundary Repair

`M5:T378:S1:NATIVE-INGRESS-BOUNDARY:OK`

## Decision

T377's keyboard/mouse failures were stale literal static checks, not direct
guest mutation. The production route is `Win32 adapter -> platform host-input
sink -> session input source -> ordered request transport -> session consumer`.
S1 changes the static proofs to require that route, rather than require the
forbidden platform-to-core direct call. No test-only input API was added.

The real dependency defect was `vm/platform/virtual_time.h` including the
composition session interface only to name a callback record. S1 moves that
record to `vm/platform/virtual_time_interface.h`; composition aliases it at
its public session boundary while platform no longer depends on composition.

## Verification

- `verify-keyboard-ingress-boundary`: passed.
- `verify-aux-mouse-boundary`: passed.
- `verify-dependency-dag`: passed with zero allowlisted migration edges.
- `verify-vm-media-atomic-save`: passed after recognizing FDD paired raw/sidecar
  persistence rather than incorrectly requiring a single-file save.
- `vm-keyboard-host-ingress-smoke` and `vm-platform-win32-keyboard-scan-smoke`:
  passed.
- Full current gate was replayed; all T378-owned checks pass. The new neutral
  platform interface changes T345's generated direct-compilation classification:
  `vm-platform/virtual_time.c` is retained explicitly as a mixed/inherited
  production residual, and the verifier's source-of-truth counts are updated
  to the regenerated 48/54 matrix. This is an ownership-ledger update, not a
  behavioral or timing change.

## Transfer

Model-339 remains not L3-ready until the queued independent re-audit repeats
the complete requirement matrix on this corrected gate baseline. This repair
does not enable Model-339 AUX/IRQ12, change its timing contract, or accept a
later machine profile.
