# M5 T381: NXVM Console Redesign And Frozen YAML Session Profiles

## Task Record

T381 makes YAML files beside the NXVM executable the sole session-creation
input and replaces the implicit default-session Console path with explicit
zero-or-more session management.  It follows the closed T380 corrective work
and precedes all DeskPro Model 40 candidates.

## Delivered Scope

### S1: Frozen configuration catalog and zero-session Console

S1 owns the production YAML snapshot catalog, startup selection interaction,
current-session command boundary, and focused product proof.  The owner has
approved the narrowly required core session-manager semantic change: it may
start empty and close its final session.  No core machine, device, CPU, memory,
or timing behavior may change.

## Closure Audit

P1 `9b6a86c6` implements the frozen YAML catalog, zero-session lifecycle,
strict Console command boundary and bounded floppy media flow.  Coordinator
review confirms the sole core change remains in generic session-container
semantics, no core-machine source changed, and the artifact/current/specialized
gates pass.

## S2 Governance Correction

The later status-only P3--P6 commits did not change the S1 delivery, evidence,
or artifact.  They nevertheless left `states/CURRENT.md` with a retained
`T381 S1` progress row while its prose stated that T381 had closed.  The
machine-readable state convention treats that row as an open numeric task.

Corrective S2 therefore restores the one task-level T381 closure row and
removes the stale S1 progress representation.  It preserves all immutable
commits: P1 remains the implementation delivery, P2 remains its original
closure attempt, and P3--P6 remain historical status-only corrections.  The
T381 artifact remains `vm-0-5-0381`; its recorded SHA-256 is rechecked without
rebuilding because no runnable source or build input changes.
