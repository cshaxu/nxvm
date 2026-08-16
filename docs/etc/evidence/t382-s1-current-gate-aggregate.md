# T382 S1: Current-Gate Aggregate Liveness

`M5:T382:S1:CURRENT-GATE-AGGREGATE:OK`

## Result

`run-current-smokes` and `run-current-fast-smokes` retain their existing
CTest selections, per-test timeout, labels, dependencies, and parallelism.
Their sole aggregate invocation is now owned by
`RunCurrentSmokeAggregate.ps1`, which delegates process lifetime to the
bounded Windows Job Object runner. The runner captures CTest output, reports
a deterministic deadline or failing-exit diagnostic, and uses one job-object
termination boundary for the root CTest process and every descendant.

The owner removes `Testing/Temporary/LastTest.log.tmp` only after the owned
process has exited. This preserves the CTest failure output supplied by the
runner while preventing the stale temporary log from blocking the next run.

## Aggregate Sweep

| Entry point | Disposition |
| --- | --- |
| `run-current-smokes` | Full current selection, PowerShell aggregate owner. |
| `run-current-fast-smokes` | Same aggregate owner with the existing `media` exclusion. |
| `current-gates-gcc` | Preset remains the composition of full smoke plus specialized gates. |
| Individual `current.*` CTest cases | Retain their existing 30-second CTest timeout; they are children of, not alternatives to, the aggregate owner. |

On a host without PowerShell, both aggregate targets fail explicitly instead
of using an unbounded direct CTest fallback. A future non-Windows support
admission must supply an equivalent process-tree supervisor and an abnormal
child proof before it can re-enable these aggregate targets.

## Verification

- Fresh GCC/Ninja configuration in `build/t382-s1-gcc` succeeded.
- `verify-t382-current-gate-aggregate` creates a parent that starts a
  deliberately long-lived child. Its two-second deadline produces the
  deterministic marker above and proves that the child PID no longer exists.
- The complete `run-current-smokes` selection completed successfully with 250
  registered current tests; no `LastTest.log.tmp` remained.
- `verify-current-specialized-gates` passed after the S1 marker was added to
  the specialized set; the current-gate separation verifier retained
  `full=250`, `media=15`, and `non-media=235`.
- Documentation governance and `git diff --check` passed before P1; the
  coordinator repeats them against the pushed actual change before acceptance.

## Artifact

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0382.exe` | `07EB0AE2C557CE478DFD8D383D455AD1FF1C5CCDA2FEA1D8815B77920D628910` |

No guest media, ROM, or host-local asset is committed.
