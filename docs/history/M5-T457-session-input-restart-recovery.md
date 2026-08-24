# M5 T457: Session Input Restart Recovery

T457 owns the current-product F9-stop then `START` keyboard-input regression.
Its one scope is the shared VM session/platform lifecycle: diagnose the
first/second input epochs, repair their owner if necessary, and retain an
end-to-end regression without changing guest keyboard semantics.

## Accepted Progress

S1 is accepted at `dd82137c`. The coordinator's actual-change review confirms
that both host adapters retain the existing run-handle report as the sole
F9-stop path and now return before ordinary key submission. The focused
two-epoch regression proves F9 is not inherited by the second session epoch
and that a subsequent `A` reaches the existing KBC endpoint. The configured
0457 product, documentation governance and 293/293 current-gate replay pass.
The [S1 evidence](../etc/evidence/t457-s1-session-input-restart-recovery.md)
records the owner/caller sweep, artifact hash and minimalism review.

## Closure

T457 closes the reported 0448 F9-stop then START keyboard failure. It removes
the sole invalid continuation that let a host-control key enter guest input,
without changing guest keyboard semantics or adding a lifecycle/input route.
The retained [proposal](M5-T457-session-input-restart-recovery-proposal.md)
and S1 evidence preserve the scope and proof. No residual in-scope variant
remains: Windows and Linux are the complete F9 reporter sweep; KBC, PIC,
request transport and Console behavior are intentionally unchanged boundaries.

## Corrective S2 and Reclosure

The owner reopened the latest closed task on 2026-08-24 to require that its
current developer executable be an optimized build with debug information and
that unattended internal trace work not slow ordinary execution.  S2 is
accepted at `19ec9fc3`: `current-gcc` now owns one isolated RelWithDebInfo
publication tree, while the existing Debug tree remains the current-gate
route.  A pre-link guard prevents every other build type from copying the
current artifact to `build/output`.

The correction retains the production debugger, including trace, pause, step,
break/watch and recording, and retains CPU fault diagnostics.  It compiles out
only automatic Core trace event recording in the optimized artifact.  The
full Debug gate exposed and corrected the stale F9 test assertion that still
expected host F9 to enter the guest queue; the product behavior was already
correct.  The rejected retirement-observation fast path is not retained,
because timing selection consumes that state.

The rebuilt `nxvm_0_5_0457.exe` is SHA-256
`D2351D8940209DBB9BAB82FDA0AB33155223F00EDB7B1C74B59B210C855B5E36`.
Focused optimized debugger tests pass; the Debug current gate passes 293/293
in 105.09 seconds; and documentation governance passes.  The retained
[corrective proposal](M5-T457-optimized-artifact-trace-audit-proposal.md) and
[S2 evidence](../etc/evidence/t457-s2-optimized-artifact-trace-audit.md)
preserve the consumer sweep and artifact proof.  No further in-scope gap
remains.
