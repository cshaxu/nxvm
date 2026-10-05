# T545 S3 Lib Test Completion

Baseline c1f864e0b; the owner admits the continuing four-package repair objective.
Shared test code is the implementation target; NXVM records are separate.
Production files and public ABI are unchanged. This is a new authorized test
revision, not an assertion of equality to the historical SoftPC import.

## Coverage Universe

All public Lib contract families: Types vocabulary; Base clock/process/sync;
logical Console and binding; Console broker; KVM frame/event/input/hotkey,
mailbox/worker/component, Console and Window; Audio stream; Storage file and
medium. Review actual source and local assertions; reference counts alone
cannot accept a contract. Platform fakes prove algorithm contracts, not native
Linux availability. Native Windows tests retain their desktop/resource policy.

## Initial Findings

Medium replacement and byte-count observation lack direct test/lib proof.
The existing storage test already owns file bytes, failure injection and live
allocation accounting, so extend it rather than adding a fixture/framework.
Replacement must transfer ownership without destroying either object, reject
invalid aliases/output state without mutation, accept empty/eject cases and
preserve each backing mode. Destruction must still consume a failing close.

## Contract Inventory And Local Proof

The inventory follows public contract families and the implementation mechanisms
that own their failure/lifecycle behavior. The following are actual assertions,
not an inference from API-name searches. Test names below denote files in
test/lib; no App fixture supplies these proofs.

| Owner / contracts | Direct local proof and disposition |
| --- | --- |
| Types vocabulary, atomics, clock | types_contract checks width/status/pointer preservation, atomic operations, monotonic clock and invalid output; this batch adds overlapping move, bounded comparison, character/substring search and tokenization. Thin CRT wrappers retain CRT preconditions, not invented NULL guarantees. |
| Base process directory | process_directory checks executable directory discovery and insufficient capacity without corrupting caller output. |
| Base mutex/event/task/wait | base_sync_ownership checks manual/auto reset, cancellation, failed creation/signal/reset/join and retained resources; console_blocking_gate checks blocking ownership. linux_wait_contract checks initialization cleanup, spurious waits, monotonic deadlines, wait failure and cancel/join. This batch calls public sleep/yield through the existing Linux fake, including interrupted sleep retry. |
| Logical Console and binding | lib_console checks copied output binding, source generation rejection, event/value delivery, text geometry/palette/Unicode and absent sink. lib_console_event_gate checks callback quiescence before detach. lib_console_io_contract checks retained buffer coverage, invalidation/retry, shrink clipping and cooked/raw restoration, without font/viewport policy changes. |
| Console broker | console_broker covers exclusive Current ownership, pending replacement cancellation, four raw/cooked transitions, preflight rollback and terminal retirement failure. console_broker_cancel checks blocked reader cancellation and completed-line race; reader_failure checks raw/cooked I/O errors, stopped/stale suppression, repeat counts and UTF-16 records; display supplies native activation proof. |
| KVM frame/value ABI | kvm_frame_copy checks text/graphics validation, row stride, visible bounds, character banks, copied active payload and failure preserving destination. win32_window_bounds checks sizing/aspect/work-area behavior. Console retirement probe checks 80x50 last cell and cursor normalization; Window capture probe checks cursor/frame damage. |
| KVM input, source, hotkey | win32_keyboard, kvm_keyboard_lifetime and kvm_input_admission check physical-key identity, orphan breaks, repeats, modifiers, Unicode, reset/focus, frozen prefix admission and failed delivery. This batch adds direct source-value assignment and held-key observation/miss assertions; no product hotkey meanings enter Lib. |
| KVM mailboxes/component/worker | kvm_component_contract and kvm_leaf_control_capacity check copied FIFO capacity, terminal STOP admission, identity exhaustion and failure reporting. mailbox_selection checks allocation cleanup and one selected notifier. This batch adds checked component frame publication, rejected publication after retirement, idempotent mailbox close, rejected new admission and draining previously accepted state. Worker STOP priority remains checked separately by shutdown/input/retirement probes. |
| KVM native Console/Window | console_retirement_barrier, window_retirement, window_modal, window_capture_contract and shutdown_failure check activation, freeze/capture, native wake, release and failed join retaining ownership. Public leaf frame validation is covered via leaf publication, not duplicated merely to obtain another API-name hit. |
| Audio stream/backend | audio_stream checks queue pressure, flush tail, inactive writes, cancellation with accepted prefix, terminal wake failures and destruction retry. audio_win32_platform checks all supported rates/channels, padding and partial delivery, exact remaining samples, all initialization cleanup stages and worker-owned COM/endpoint release. No audible/physical speaker assertion is required. |
| Storage file/medium | storage_file_writer_binary checks binary reads/writes, owned output, sparse 4KiB pages, range/index overflow, partial writes, allocation failure, readonly/discard semantics and consumed-close failure. This batch adds truncate/append byte identity, empty/invalid requests, close/allocation failures, a 3x3 DIRECT/READONLY/OVERLAY lock matrix, byte-count and replacement ownership/failure atomicity. storage_position checks offsets above 4GiB without a huge fixture. linux_storage_contract checks shared/exclusive nonblocking lock calls and cleanup; Linux-only storage_lock checks real descriptor/inode locks when built there. |

All registered owner-local tests remain. No old assertion is relocated or
removed in S3. All additions reuse six existing C tests and one fake header;
no test executable, public API, production path or generic fixture is added.
The same-handle native Windows lock matrix proves OS sharing enforcement for
independent opens, not a new interprocess timing experiment. Linux fake tests
do not claim native Linux execution. The inventory is contract proof, not a
measured 100-percent branch-coverage claim.

## Verification And Acceptance

Standalone Release builds succeed on x64/x86 with strict warnings. Background
suite x64 passes 48/48; x86 initially passes 47/48 with a manifest mismatch
caused by the final component test initializer edit during verification. After
refreshing its hash, both widths pass the component and manifest checks. Final
storage additions also pass both widths. Both native desktop groups pass 3/3
(4.24s x64, 3.91s x86). Thus all 51 registered standalone cases have current
passing proof, including reruns of cases whose source or manifest changed
during verification. No failure is silently discarded.

Final full repository units pass 531/531 on each width: 65.83s x64 after the
last changed test was rebuilt; 66.32s x86 through run-unit-tests. An earlier
x64 complete run passed before the final lock-matrix addition and is not used
as its proof. Nineteen source/test manifest, Types, corpus and ownership checks
pass per width, covering all eight manifests. Standalone background runs also
exercise boundary/DAG negative cases and Types-layout self-tests.

Commands: cmake -S test/lib with the receiving x64/x86 MinGW compilers and
Release; cmake --build each standalone root; ctest -LE desktop -j8, then
ctest -L desktop -j1. Receiving roots use run-unit-tests (x64 final ctest -L
unit after affected targets rebuilt) and the explicit 19-check selector.
Documentation governance and git diff --check pass.

Sequential executor/coordinator review checks additions against owner source,
retains original assertions, preserves consumed-close behavior and output
failure atomicity, and confirms no App/foreign-test dependency. No discovered
production defect is papered over with a new API or weakened assertion.

Production inputs are unchanged, so current 0545 PC and 0043 MyNES EXEs do not
require rebuilding. External integration remains the original S7 gate.

Actual Shared diff: six existing C tests plus one fake header, +198/-3 lines
(net +195); README +5/-3; manifest +8/-8. No CMake registration changes, deleted
tests, production changes or executable changes. Target-separated delivery:
Shared S3 P1 contains only test/lib; NXVM S3 P2 owns this evidence and acceptance.
