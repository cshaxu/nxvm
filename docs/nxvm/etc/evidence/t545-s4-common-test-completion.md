# T545 S4 Common Test Completion

Baseline 3b059030f; Shared test-only work under the approved four-package goal.
Production/API and deployed artifacts remain unchanged.

## Initial Findings

Existing Session frame/monitor tests execute production reducers with neutral
Machine/UI doubles, but mainly construct private stack state. Complete the
real public create/bind/destroy and event ingress proof in the same owner-local
fixtures. Queue failure tests already own allocation and terminal delivery
mechanisms; they do not alone prove the public Session envelope.

Inventory: Machine input/control FIFO, executor ownership and synchronization;
UI composition/event mapping, resources and frame lifecycle; Session reducer,
reconciliation, monitor/provider policy and public lifecycle/ingress. Actual
assertions determine coverage, not function-name searches. All inventory and
verification results determine acceptance; API-name hits alone are insufficient.

## Owner-Local Contract Inventory

| Contract family | Local assertions / disposition |
| --- | --- |
| Machine construction, executor and lifecycle | common_machine uses a neutral real-thread driver for reset/start/pause/resume/stop and sink ownership. machine_wait injects creation, wake/reset/signal/wait failures and verifies one terminal completion, retained/cancelled requests and resource lifetime. No CPU/register interpretation enters this package. |
| Machine synchronous operations | common_machine and machine_wait cover paused Debug lease generation, copied byte requests/responses, capacity errors, driver failure, cancellation, state read/write completion and media mode/path propagation; product image/register semantics stay outside. |
| Machine frames | common_machine verifies ready publication, stale-run rejection, immutable completed snapshots and frame failure terminating live execution. This batch directly observes published run identity. session_frame adds publication copy of serial/text maps, graphics not overwriting text-only maps, self-copy and invalid payload preserving existing state. Lib retains pixel/frame-format validation ownership. |
| Machine input FIFO | input_queue_mouse_fifo preserves relative events in order; input_queue_ownership checks failed initialization/disposal and ring wrap; sync checks full capacity, clear and paired producers/consumer. These do not simulate guest input hardware. |
| UI ownership/composition | composition executes actual common_ui with neutral Lib presenter/broker doubles. It already covers frame-status caching/retry, serial wrap, 80x50 characters, callback generation, mouse routing and failed teardown retaining callback owners. This batch adds all four initialization failure stages and direct monitor/mouse control propagation, plus cooked/rejected line and I/O failure event mapping through the registered callback. |
| Session public lifecycle/ingress | session_monitor now calls real create/bind/destroy: required provider validation, allocation/queue failure cleanup, single UI binding and actual loop exit. Its injected open callback sends all five lifecycle requests through the public run loop; distinct fake API counters verify correct dispatch, not just any lifecycle call. Unknown requests fail without dispatch. All six UI kinds, both component identities, runtime/frame completion values and caller mutation after enqueue have local proof. Existing queue_failure independently owns partial queue initialization and terminal allocation failures. |
| Session input admission | control_reconciler_integration and physical_key_identity prove generation rejection, source-local physical identity, frozen-input admission and paired release/retirement without delivering stopped guest input. No native Lib hotkey algorithm is reimplemented. |
| Session presentation/reducer | presentation_plan, reconciler and control_state_matrix cover Console/Window x console_control, first-frame readiness, one in-flight action, actual completion versus desired target, close/pause/resume/restart and raw/monitor handoff. session_frame covers skipped/latest frame notification, generation changes, serial wrap and stale/duplicate notifications. |
| Monitor/provider contract | session_monitor retains prompt admission separate from notices, completed-but-unconsumed lines, cancellation failure, accepted/rejected input, long output normalization, writer failure and failed indefinite wait without retry. Product command grammar remains injected; Common receives no App CLI code. |

No original assertion or registration is removed. Four existing test files
receive new checks, with existing doubles reused; no production API, framework,
App fixture, foreign test helper or test target is added. Strict standalone
builds need src/common, inward src/lib and neutral root CMake helpers, not
test/lib/x86/IBMPC or any App. Native thread proof is Windows-only; no native
Linux claim is made.

## Verification And Acceptance

Standalone Common builds succeed on both widths with C11 strict warnings.
Both complete 20-case suites initially pass; the four finally changed cases
and current manifest also pass individually on both widths. The initial x86
configuration used a nonexistent make path; selecting the installed MinGW
make fixed configuration without altering test/production code. Final x64
receiver units pass 531/531 (63.45s), with all four affected targets rebuilt
after their final source edits. Six Common boundary/Types/corpus/manifest and
negative checks pass per width, with the final changed manifest rechecked.
Final x86 receiver units also pass 531/531 (80.71s); both complete runs use
current-source affected targets. Twenty standalone cases per width have
passing proof, including final reruns of all subsequently changed cases and
manifests. Documentation governance and git diff --check pass. Sequential
executor/coordinator review accepts direct own-contract proof and no production
change; this is not a measured full-branch or hardware-qualification claim.

Actual review retains all original test assertions and registrations. The
broker double now returns checked INVALID_STATE for a wrong Current object,
rather than aborting inside the double, so UI propagation is observable. Its
successful caller assertions still reject incorrect binding. New failure
switches affect only test doubles and are reset before existing scenarios.
Source changes are four existing test files, +313/-12 (net +301); README +3/-1
and manifest +6/-6. No production/build/executable inputs change, so artifacts
are not rebuilt. Shared P1 contains test/common only; NXVM P2 owns acceptance
and this ledger. S4 closes with target-separated delivery, while S5-S7 remain.
