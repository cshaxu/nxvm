# M5 T534 App/Core Code-Quality Remediation

The owner admitted T534 on 2026-09-21 to repair the whole-project quality
audit's confirmed App/Core defects, repeating the audit until the frozen
App/Core corpus has an explicit disposition for each finding.  The retained
candidate proposal is [App/Core code-quality remediation](../proposals/m5-app-core-code-quality-remediation.md).

## S1: Remove Unsupported Recorder

S1 removes the unconnected instruction recorder as a complete mechanism.  It
does not replace it, add a tracing abstraction or modify Debug semantics.
Closure requires no production or test recorder route, no stale build/policy
claim, complete repository-only unit proof and actual-diff review.

P1 `4fc6828d` removes the command, App object, copied observer contract,
dedicated smoke and build verifier. Actual-diff review confirmed that retained
Core Debug observation remains private to the debugger execution path. Fresh
repository-only unit proof is 334/334 passing; documentation governance and
`git diff --check` pass. S1 is closed.

## S2: Presentation Single Path

S2 makes Machine-frame conversion failure-atomic and removes the duplicate
Core presentation mailbox. Integration probes observe the one Common-frame
production route through a test-only capture adapter. Closure requires 333/333
repository-only unit and 20/20 external integration proof plus actual-diff
review.

P1 `32d3585d` removes the Core mailbox and makes conversion failure-atomic.
Focused proof, 333/333 repository-only unit and 20/20 external integration
pass. S2 is closed.

## S3: Direct Profile Plans

S3 removes the runtime Profile-inheritance/provenance mirror and returns
PC/AT board controller construction to Profile. Machine continues to own media
objects and registry binding, but no longer provides a generic board
materializer callback. The direct value contract validates each effective plan;
tests assert effective configuration and topology rather than ancestry text.

Closure requires a whole-corpus caller sweep, focused retained-board proof,
complete repository-only unit proof and actual-diff review.

P1 `2a667f8f` deletes the resolver graph, per-field provenance and generic
Machine materializer.  Direct Profile contracts retain the former validation
rules, including zero-ID, overlap and duplicate-line rejection.  The caller
sweep is empty; 333/333 repository-only unit and 20/20 external integration
pass; documentation governance and actual-diff review pass.  S3 is closed.

## S4: Canonical INI File Route

P1 replaces App's direct stream loader with the existing Lib storage reader,
deletes every unused root file-operation wrapper, and preserves a root path's
leading separator while resolving relative media.  The retained `STD_FILE`
alias remains solely as the parameter type of existing diagnostic output.
Focused INI proof and 333/333 repository-only unit pass.  S4 is closed.

## S5: Repeat Audit

The repeat audit reviewed App composition/commands and Machine lifecycle,
driver and Debug-adapter boundaries.  It found CQ-7: three public lifecycle
entry points have no caller and no unique behavior after Common lifecycle and
x86 Debug execution plans became the sole routes.  S6 receives their complete
removal; S5 itself makes no production change.
