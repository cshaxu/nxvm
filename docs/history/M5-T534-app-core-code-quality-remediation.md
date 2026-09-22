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

## S6: Lifecycle Path Removal

P1 `721bffb9` removes the uncalled polling pause, empty step and ignored-reason
lifecycle APIs.  The remaining pause route directly requests Common ownership;
x86 Debug continues through its existing execution plan.  The whole caller
sweep is empty and 333/333 repository-only unit pass.  S6 is closed.

## S7: Repeat Audit

The next source-level audit found CQ-8: App's staged Machine/Control/UI
composition leaves half-built state after a failure.  S8 must make each stage
failure-atomic without adding a parallel composition route.

## S8: Failure-Atomic App Composition

P1 `c736e749` makes App publish Machine, Common Machine, Control and UI only
after the respective stage succeeds.  A repository-only composition test
forces every configurable stage failure, verifies the failed stage has no
published App state, and retries the same App successfully.  The full unit
suite passes 334/334 in 19.55 seconds.  S8 is closed; S9 repeats the full
App/Core quality audit.

## S9: Repeat Audit

S9 re-read all tracked App/Core declarations, implementations and direct
repository-only callers for stale public operations, duplicate state/output
routes, host leakage and partial construction.  The recorder removal remains
complete: no App/Core production, test or build route survives.

The audit found two bounded repair batches.  CQ-9 is a subtraction-only media
cleanup: the public fixed-disk insertion stub always fails, floppy ejection
pretends to accept a path it ignores, and the FDC retains an uncalled direct
stdout dump.  CQ-10 is not safe to delete mechanically: the CPU still emits
watchpoint text directly while the retained Debug CLI has an existing copied
result presentation route whose `watch_hit` fields are never populated.  The
next S removes CQ-9; the following S must populate the established copied
observation before removing the Core stdout path.  No other App/Core
production owner, direct host API route, or composition rollback hole was
found in this repeat sweep.

## S10: Media Facade Subtraction

P1 removes the fixed-disk insertion API that could only fail, changes floppy
ejection to an exact no-argument operation, and deletes the uncalled FDC
stdout dump.  The retained fixed-disk setup remains private to Machine
initialization; removable media still changes only through Common.  The
affected smoke now creates its ephemeral blank image with the existing Lib
binary writer, so a clean compile no longer relied on absent root file wrappers.
Focused proof passes from a newly linked executable, and the full
repository-only unit suite passes 334/334 in 24.08 seconds.  The exact retired
symbol sweep is empty.  S10 is closed; S11 completes the existing copied
watchpoint result path before retiring direct CPU stdout output.

## S11: Watchpoint Result Path And Fresh-Test Repair

S11 repairs the incomplete watchpoint migration rather than hiding its direct
CPU text.  Core records the first read/write/execute match in the retired
instruction observation and requests a debug pause, distinct from the
machine-stop/reset request.  Machine copies that observation through the
existing x86 result response, so the retained Debug command text is the only
user-facing report owner.  Focused proof covers all three Core watch kinds and
the Core-to-x86 copied result mapping; the CPU source sweep has no active
`STD_PRINTF` call.

The required fresh complete unit build also exposed CQ-11: two media smokes
still named deleted root file wrappers, and generated-result runners did too.
Media fixtures now use the retained Lib reader/writer.  The generator runners
write only their CMake-provided build artifacts through ISO-C streams; they are
test-only output endpoints, not an App/Core host-file route.  A full fresh
target build and 334/334 repository-only unit tests pass.  S11 is closed; S12
repeats the same App/Core audit.

## S13: Dead Boundary Subtraction

The subsequent declaration/caller audit found CQ-12: several published
helpers had no consumer beyond their own definition.  S13 removes the CPU
segment switch wrapper, unused display mode notifier, FPU naming/busy helpers,
unused firmware A20/stop capability, timing manifest count accessor, KBC
output-port callback seam, and plan-level memory-device container.  The latter
two removals include their dormant state and application loop, not only their
public declarations.  Existing Core-local loaders, KBC A20/reset behavior,
FPU dispatch, direct memory mapping and timing selection remain their sole
live owners.

The complete declaration sweep has no remaining CQ-12 symbol, the full clean
unit target builds, 334/334 repository-only unit tests pass, and documentation
governance plus whitespace checks pass.  S13 is closed; the repeated T534
audit continues with the next App/Core mechanism family.

## S14: Truthful Composition Failures

S14 removes the App composition boundary's false `INVALID_STATE` result.  Type
status failures now return unchanged.  The one local boundary converter maps
the stable Lib classifications to the corresponding Type outcome; unexpected
Lib I/O or limit results become the existing Type fault rather than an invented
parallel status domain.  The same rollback remains in place and is executed
only after a later stage has actually allocated or bound an object.

The App composition smoke now asserts the distinct configuration, creation,
driver, Common-create, bind, session and UI failure classifications, as well
as its existing retry checks.  It passes independently; complete unit and
documentation evidence is recorded with the S14 commit.

## S15: Core Result Preservation In Machine

S15 applies the same failure-boundary rule to VM Machine.  Memory
reconfiguration, reset-vector capture and information capture now return the
actual Core status after their own local validation.  They no longer rewrite a
Core allocation, mapping or lifecycle result as a generic Machine state error.
The reset-vector smoke also proves invalid Machine arguments remain local
`INVALID_ARGUMENT` outcomes.  A static sweep has no remaining direct
Core-failure-to-`INVALID_STATE` rewrite in App or Machine.

## S16: Truthful Fixed-Media Construction

The next repeated audit classified CQ-15.  HDD is an initialization-owned fixed
medium, but its provider advertises removable/change-detectable capabilities,
its internal detach operation accepts and ignores a pathname, and requested
blank-disk creation silently leaves a machine without the requested medium if
allocation fails.  S16 removes the false parameter/capabilities and carries
that owner-local allocation result through the existing Machine construction
boundary.  The retained startup attachment route remains private; no HDC
protocol or Lib Storage interface changes are allowed.

P1 makes the HDD provider report only geometry/format/read-only facts, removes
the ignored pathname from its internal detach operation, and makes blank-disk
creation return success or failure.  Machine now rejects a requested blank disk
with `TYPE_STATUS_NO_MEMORY` if its allocation cannot complete.  The provider,
direct/readonly/overlay and Machine construction probes pass, along with the
334-case repository-only unit suite and documentation governance gate.  S16 is
closed.

## S17: Truthful Floppy Detach Contract

The S16 similarity sweep then found the equivalent false input in FDD detach:
the operation takes a pathname it always ignores.  S17 removes that input from
the private FDD owner and every caller.  FDD's blank-medium allocations already
occur in `initialize_with_geometry`, whose result reaches Machine construction;
the S verifies that this is the sole retained allocation boundary rather than
inventing a second FDD creation status path.

P1 makes all FDD detach callers use the single parameterless operation.  The
provider smoke proves empty initialization, blank-media activation and detach;
direct/readonly/overlay, FDC-port and Model40 FDC smokes pass.  The complete
334-case unit suite and documentation governance gate pass.  S17 is closed.
