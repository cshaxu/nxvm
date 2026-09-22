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

## S18: Core Reset Failure Preservation

S18 removes both Core run-path rewrites of a cold-reset failure to generic
`FAULT`.  Firmware reset remains the status owner; run now returns its exact
result.  The firmware capability smoke proves `UNSUPPORTED` survives a pending
stop reset, and the 334-case unit suite passes.

## S19: Boundary Status Preservation

The next repeat audit found two remaining status masks. App composition
rewrites a Machine creation result to `INVALID_STATE`; Machine's x86 Debug
adapter rewrites every Core Debug result to the same Lib state error. S19
must preserve the former exactly and use one complete local Type-to-Lib
mapping for the latter. It does not change Debug grammar or any public
contract.

P1 preserves Machine creation status in App and replaces the Debug adapter's
per-operation generic-state rewrites with one complete private Type-to-Lib
mapping. The focused composition and Debug mapping smokes, all 334
repository-only unit tests and documentation governance pass. S19 is closed.

## S20: Runner Failure Propagation

The next repeat audit found CQ-20: the bounded Machine runner treated a
non-fault Core-run failure, pacing failure or waiting-advance failure as a
normal stop.  Its Common driver therefore returned success and Common reported
`STOPPED` rather than `ERROR`.  S20 keeps Core fault diagnostics distinct and
uses one private Machine-owned `runner_failed` fact solely to adapt every
abnormal runner exit to Common's existing boolean driver result.

P1 sets that fact for Core fault, non-fault run, pacing and waiting-advance
failure, while the ordinary Core stop-request path stays a normal stop.  The
new Common-bound smoke forces a non-fault Core `INVALID_STATE` after a paused
safe boundary and proves `COMMON_MACHINE_ERROR`; the existing fault-outcome
smoke remains green.  A complete rebuild and 335/335 repository-only unit
tests pass, as do documentation governance and whitespace checks. S20 is
closed.

## S21: Direct Copied-Frame Construction

The next repeat audit found CQ-21: after it had already validated the input,
the Machine display adapter allocated a complete temporary Common frame,
populated it, copied it into its caller-owned destination, then freed the
temporary on every published display update.  S21 removes that redundant
allocation and full-frame copy.  Every fallible source validation remains
before the first destination write, so an invalid event still leaves the
previous copied frame byte-identical.

The retained conversion contract and presentation route are unchanged: valid
text and graphics events populate the supplied Common frame exactly once.
The focused frame smoke covers both success shapes and unchanged rejection;
the 335-case repository-only unit suite passes.  A complete default build also
exposed two unrelated integration-test compile debts (`STD_FPUTS` after the
root facade retirement and a removed `vm_session_request.profile` field). They
are recorded for the continuing audit rather than hidden in this focused
presentation repair. S21 is closed.

## S22: Integration Contract Catch-up

The default build's next errors were stale integration-only references to six
retired root output facades and one removed runtime `profile` request field.
S22 replaces the test diagnostics with the retained test-local formatter and
reads the frozen Machine profile kind for the Model 40-only probe. No
production facade or runtime profile selector returns.

All three affected integration targets compile, the lifecycle integration
probe passes, and the complete 335-case repository-only unit suite remains
green. The complete default Ninja target itself repeatedly deadlocks without
a compiler child after its known targets complete; this is retained as build
tooling evidence, not relabelled as a source failure. S22 is closed.

## S23: Failure-Atomic App Teardown

S23 repairs App teardown against Common's existing shutdown contract. A failed
worker join retains the Common object and its callback contexts, so App now
returns the mapped failure immediately and leaves its complete object graph
available for a later retry. Only successful shutdown enters UI/session/Common
destruction and Machine unbinding. The composition fake boundary proves the
failed attempt destroys nothing, followed by successful one-time cleanup on
retry. The 335-case repository-only unit suite passes. S23 is closed.

## S24: Reset Failure Propagation

The repeat runner audit found one remaining abnormal terminal branch. A reset
requested through the retained direct-control boundary records the Core reset
failure, but its runner continued through the normal-stop result path. S24
routes that exact branch through the existing Machine-owned `runner_failed`
fact. It neither adds a lifecycle state nor changes Common's contract.

The retained runner smoke now also swaps only the already-configured firmware
reset callback after cold start, requests an active reset, and proves that
`TYPE_STATUS_UNSUPPORTED` produces `COMMON_MACHINE_ERROR`. The existing
non-fault Core-run probe remains in the same smoke. All 335 repository-only
unit tests pass. S24 is closed.

## S25: Direct Constructor Output Ownership

The direct Core create entry validated configuration before clearing its output
pointer. That differed from Core plan construction and every peer owner-local
creator, all of which establish the output contract before evaluating the
remaining input. S25 moves the null assignment immediately after validating
the output parameter; no construction or configuration mechanism changes.

The retained time smoke now supplies a non-null sentinel before its invalid
time-axis configuration cases and proves it is cleared on the first failure.
The complete 335-case repository-only unit suite passes. S25 is closed.

## S26: Complete Lifecycle-Owner Correction

The S6 facade removal did not remove the full defect: Machine Control still
owned a private pause-reason/step state, and several integration probes drove
that state directly. S26 removes that residual lifecycle state and its test
entry points. The bounded executor now performs a Common rendezvous immediately
after each completed Core quantum and before optional Standard pacing, so a
pending Common pause, reset or Debug completion cannot be delayed behind a
wall-clock wait.

Integration probes now start, pause, reset and resume solely through Common.
Their exact Debug work uses the established paused lease and x86 execution
plan. The timer/firmware probe additionally uses that lease to write its
real-mode programs, set a real-address breakpoint and read the outcome; it no
longer calls Core run or memory APIs beside a paused Common session. The
similar-symbol sweep finds no `VM_MACHINE_PAUSE_*` or retired direct Machine
pause/step API caller. Focused Common-driven lifecycle and timer probes pass,
as does the complete 335-case repository-only unit suite. S26 remains subject
to the T-level repeated quality audit.

## S27: Fallible RAM Fixture Construction

The repeated audit found a void Core RAM convenience initializer that allocated
16 MiB and discarded its only failure result. It had no production caller:
eighteen repository-only Core fixtures used it. S27 deletes that implicit
entry point and makes every fixture call the existing fallible initializer with
its own requested capacity. The two fixtures that intentionally use 2 MiB now
request 2 MiB directly instead of allocating 16 MiB then replacing it.

The full caller sweep is empty. All eighteen affected RAM, DMA and VADP smokes
pass after a fresh source compile/link, and the complete 335-case
repository-only unit suite passes. S27 remains subject to the T-level repeated
quality audit.

## S28: Fallible Default-FDD Construction

The next repeated audit found the same failure-masking shape in the legacy
default FDD fixture constructor.  Production already uses the fallible
geometry constructor; only two repository-only media fixtures still invoke the
void wrapper.  S28 removes the wrapper and makes those fixtures request the
same default geometry explicitly, preserving one FDD allocation boundary.

## S29: Constructor Output-State Convergence

The next repeated audit found the same output-state ordering defect beneath the
Machine boundary: Profile file-backed plan construction and Machine wrapper
constructors can reject input or fail asset loading while leaving a valid
caller-owned output pointer unchanged. S29 applies the established Core
constructor rule to the entire Profile/Machine wrapper family and proves both
invalid-input and file-backed failure cases clear the output.

## S30: Truthful INI-Load Failures

The next audit found the App INI loader combines caller validation, Lib file
loading and local document allocation into one `FAULT` branch. S30 restores
the existing status distinctions at the one App-owned boundary without adding a
second parser or changing the INI grammar.

S30 keeps the existing single load/parse path. Its only behavioral change is
the explicit status boundary: invalid App arguments return `INVALID_ARGUMENT`,
allocation failures return `NO_MEMORY`, and ordinary storage failures remain
`FAULT`. The focused smoke and the complete 335-test unit suite pass.

## S31: Truthful BYOB Asset-Load Failures

The next audit found the same status collapse in the Core Profile BYOB asset
loading family. S31 keeps one materialization path while preserving invalid
argument and allocation outcomes, and gives its private file loaders the same
clear-output-on-failure rule as public constructors.

The targeted Profile/Machine construction smokes and complete 335-test unit
suite pass. The unit-level blob argument checks remain repository-only; an
allocator-failure injection seam was deliberately not added merely to fabricate
a `NO_MEMORY` branch, because Lib already owns that allocation result.

## S32: Remove Disabled Development Trace from Production Core

The next audit found a legacy development trace allocated even though its
compile-time switch is disabled. An explicit trace-enabled compile then showed
the dormant implementation is already nonfunctional: helpers expand trace
macros without a CPU context. S32 therefore removes that dead diagnostic path,
while retaining decoder error-control flow and leaving the product debugger
intact. The focused CPU-context smoke and complete 334-test unit suite pass.

## S33: Truthful App Process Outcome

The next audit found the console entry discards every non-success outcome,
allowing the executable to return success after a failed INI load, composition
or runner loop. S33 will make the App entry-to-exit result explicit without
changing any machine or Common behavior.

S33 makes the console entry return the exact INI or composition outcome and
maps an abnormal Common runner completion to `FAULT`. `main()` always destroys
the created App graph, then returns success only if both execution and teardown
succeed. The focused App INI smoke passes; a fresh executable launched beside
no `NXVM.ini` prints the existing error and exits with code 1; and the complete
334-case repository-only unit suite passes.

## S34: Complete App Monitor-Input Copy

The next audit found the App's ordinary command route copies a valid Common
monitor line into a much smaller fixed buffer with silent `snprintf`
truncation. S34 will make the App copy boundary match Common's already
validated input capacity, retaining one parser and leaving the Debug route
unchanged.

S34 gives the App's sole copied command buffer the same capacity as its Common
producer. The buffer is not an additional input queue or parser; it remains
the one mutable copy required by tokenization. The App caller sweep finds no
other monitor-line copy, the focused command smoke passes, and the complete
334-case repository-only unit suite passes.

## S35: Failure-Atomic App Configuration Publication

The next audit found a repeated App output-state defect: configuration and INI
entry points can fail after leaving a caller's old value intact or after
directly writing only part of a new request. S35 will establish the existing
constructor rule at every public App output boundary: a valid output is first
cleared and receives a complete value only after all work succeeds.

S35 clears valid public App outputs before further validation. INI parsing
builds one local request and assigns it only after the whole document succeeds,
so malformed input cannot leak a partially parsed configuration. Focused tests
start both request and configuration outputs with nonzero sentinels; invalid
inputs leave them cleared. The complete 334-case repository-only unit suite
passes.

## S36: Core Executor-State Facade Convergence

The next full App/Core source sweep found the executor-state owner alone uses
direct C allocation and atomic functions despite the established project type
facade. S36 will substitute the existing facade only; its active/reset state
machine, callers and public interface remain unchanged.

S36 replaces each direct runtime call with its existing `STD_*` equivalent,
without adding a wrapper or changing executor behavior. The direct-runtime
sweep is empty, the focused executor-state smoke passes, and the complete
334-case repository-only unit suite passes.

## S37: App/Core Type-Vocabulary Convergence

The post-S36 sweep found the same direct-C vocabulary defect one layer wider:
the executor-state public surface and implementation retain raw `void`, `int`
and `atomic_bool`, with equivalent raw spellings in Machine Debug, the Debug
adapter, FPU conversion and App INI code. S37 replaces only those spellings
with the existing `C_VOID`, `C_INT` and `STD_ATOMIC_BOOL` facade. No signature,
ABI, state transition, parser or numeric representation may change.

S37 closes the entire confirmed equivalence set, including callback ABI
spellings and ignored-result casts. The complete raw-token sweep has no code
match. Strict C11 `-Werror` checks cover every modified source; a fresh Make
build passes all 334 repository-only unit tests in 17.67 seconds, and
documentation governance passes. T534 remains open for the next independent
App/Core audit pass.

## S38: Machine Display Callback Naming

The next audit found a single lower-prefix camel-case callback introduced when
T532 relocated the Machine display adapter. It is private, has one binding
site, and differs from every adjacent `vm_machine_*` symbol only in naming.
S38 restores that one snake-case identifier and classifies the complete
App/Core lower-prefix camel-case sweep, preserving retained original CPU/device
vocabulary rather than applying a bulk rename.

S38 restores `vm_machine_display_mode_changed` at its sole bind site. The
complete lower-prefix camel-case App/Core sweep is empty. Strict C11 `-Werror`,
documentation governance and 334/334 repository-only unit tests at `-j4` in
22.56 seconds pass. A separate host-scheduling finding from the intentionally
more aggressive `-j8` replay is recorded as CQ-38; it is not attributed to the
display rename or represented as a green parallel test result.

## S39: Event-Backed Core Machine Test Waits

CQ-38 identifies four repository-only Machine tests that independently poll
Common state with fixed one-millisecond sleeps. S39 replaces that repeated
host-scheduling-sensitive construction with one test-owned state-sink/event
helper and a monotonic deadline. It does not change Common or Machine
production behavior and must demonstrate a full `-j8` unit replay.

S39 centralizes those waits in the repository-only fixture. The four focused
tests pass in 3.79 seconds, their fixed-sleep scan is empty, and the complete
334-case `-j8` replay passes in 16.64 seconds. No production source or shared
component changed.

## S40: Machine Creation Status Contract

The post-S39 public-interface audit found that `vm_machine_create()` returns
only `TYPE_STATUS_*` values and every consumer treats it as that domain, while
its declaration and the App composition fake still declare raw `C_INT`. S40
aligns that one cross-module result contract and classifies every App/Core
creation entry before the next audit pass. It changes no status value,
construction path or ownership boundary.

S40 changes the declaration, implementation and composition fake to
`type_status`. Strict C11/Werror checks, focused Machine initialization proof,
documentation governance and a complete `-j8` replay pass 334/334 in 25.15
seconds. One preceding parallel replay failed `vm-runner-error-propagation`
once; it passed on 12 isolated repeats and on the final full replay, so it is
recorded as a non-reproducible test observation rather than misclassified as a
product defect.

## S41: App Formatting Facade Convergence

The post-S40 App/Core audit found App Command alone bypassing the retained Type
C-runtime formatting vocabulary. S41 adds the missing va-list equivalent at
that existing facade and routes its three Command formatting calls through it.
It preserves every buffer, formatter and truncation result. Strict C11/Werror,
the App-console smoke, documentation governance and the complete 334-case
`-j8` unit replay pass in 16.36 seconds.

## S42: Profile-Plan Boundary Tightening

The public Profile-plan interface was importing two Profile private headers.
Its PC/AT descriptor getter only populated an unconsumed Machine field. S42
removes that projection and makes the retained Model40 ROM observation opaque
at the public boundary; the one test that needs ROM bytes explicitly includes
the Profile-private observation declaration. Focused Model40 and Machine
initialization proof passes, as does the 334-case `-j8` unit replay in 15.39
seconds.

## S44: Dead Machine Lifecycle Facades

The lifecycle caller audit found `vm_machine_start()` and
`vm_machine_is_running()` had no consumers. S44 removes both and corrects the
one stale boot-probe comment. The runner regression and rebuilt DOS probe pass;
the complete 334-case `-j8` unit suite passes in 15.65 seconds.

## S43: Machine Reset-Outcome Cache Removal

The state-owner audit found `start_outcome` was a write-only mirror of the
status already returned by the reset path. S43 deletes its five fields and two
helpers. The direct failure result remains unchanged. Focused lifecycle and
runner-error proof plus the complete 334-case `-j8` suite pass in 15.52
seconds.

## S45: INI Ordered-Media Count Simplification

The post-S44 static-analysis sweep found that the App INI parser first requires
the requested media slot to equal the current count, then conditionally advances
that count only when it is less than or equal to the same slot. The condition is
therefore always true. S45 retains the ordered-slot guard and direct path/mode
validation, then makes the successful count advance explicit. It changes no
grammar, path, access-mode or request-publication behavior.

The focused INI smoke, complete 334-case `-j8` unit replay and documentation
governance pass. The same static-analysis sweep finds no other provably true or
false condition; only non-semantic const-pointer advice and two CPU declaration
parameter-name mismatches remain for separate disposition.

## S46: Profile-Owned Console Identity

The S45 manual App review found the `INFO` command reporting IBM PC/AT as a
literal while separately displaying the selected Profile. This gave every
non-default fixed product two conflicting board identities. S46 removes the
literal and uses the frozen Profile name as the sole machine identity. The
console unit, 5170 console-probe build, complete 334-case `-j8` unit replay
and documentation governance pass.

## S47: Dead VADP Text Configurator

The post-S46 whole-caller audit found the direct VADP text configurator was an
uncalled declaration/definition pair. Profile display construction already
uses the retained timing and glyph configuration route. S47 deletes the
obsolete setter without changing VADP state or display semantics. Focused VADP
text/status probes, the complete 334-case `-j8` unit replay and documentation
governance pass.

## S48: Profile-Owned Identity Mapping

The post-S47 ownership audit found the fixed Profile-name switch in Machine,
despite it depending only on immutable Profile selection. S48 moves the one
mapping into Profile selection and changes its App and test consumers without
changing any name. Focused XT Profile and App-console probes, the complete
334-case `-j8` unit replay and documentation governance pass.

## S49: Retired Parallel Console Test Removal

The post-S48 test audit found the nominal App-console unit target exercised an
entirely test-owned command parser/lifecycle state machine rather than
production App Command. S49 removes that target and its three-file support
corpus. The real Common-driven console integration target remains registered
and builds. The remaining 333 repository-only unit tests pass at `-j8`, as
does documentation governance.

## S50: T534 Current Artifact Identity

The task-exit audit found runnable T534 source still attached to the inherited
0533 artifact target. S50 advances the sole current target and all build
presets to 0534, then produces the x64/x86 artifacts for default, IBM 5170,
XT and Model 40. The 333-case unit replay and every configured external
integration suite pass. The source revision, hashes, PE architecture and
stripping verification are retained in the
[artifact record](../etc/evidence/t534-s50-artifact-verification.md).

## S51: App Read-only Interface Qualifiers

The final static review identified exactly ten App inputs whose pointees are
only read: three monitor-command handler contexts, four App composition getter
inputs and three post-trim INI tokens. S51 qualifies those inputs without
changing returned handles, parsing, object ownership or callback behavior.
The two command contexts that submit a command or receive a runtime event stay
mutable because those paths update control/debug state; INI values stay mutable
because parsing splits and trims them. Core output buffers and the reported CPU
declaration parameter-name differences are explicitly classified as analyzer
limitations, not disguised const opportunities. Focused INI/composition tests
and the complete 333-case `-j8` repository-only unit suite pass.

## S52: Current-source Artifact Reproducibility

S52 rebuilds the default, IBM 5170, XT and Model 40 products for both x64 and
x86 from S51 source. All eight are deployed only to their profile-specific
ignored directories beside NXVM.ini; PE inspection confirms architecture and
the absence of debug sections. Default serial integration is 20/20 and the
profile-specific sets are 5170 3/3, XT 1/1 and Model 40 3/3. A preceding
default parallel run exposed a timer-firmware flake; its isolated and serial
runs pass, but the finding is retained for repair before task closure.
