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
