# M5 T526: NXVM Canonical-Library Integration Repair

## Active Packet

- **Identifier mode:** Continuation
- **Owner admission:** owner rejected closure and approved S8 to adopt the
  current owner-controlled SoftPC `src/lib` corpus without an NXVM fork.
- **Current subtask:** S8 - canonical SoftPC library refresh and consumer
  repair.

## S1 Contract

The direct reproducer is a 25-row text frame whose rows from approximately 13
through 25 have invisible text in both Console and Window.  Core produces
8-bit attributes while `ux_frame` stores 16-bit attributes; a byte-counted
copy populated only the first 1,000 of 2,000 UX elements.  S1 replaces that
invalid representation copy with explicit element conversion and audits the
remaining Core-to-UX copied fields for the same class of width/count error.

The regression must prove the final 80x25 cell's character and attribute
survive conversion.  `src/lib` is excluded from modification: SoftPC's working
canonical presenter proves this is a product-adapter fault.

## S2 Contract

The canonical lib already turns `WM_CLOSE` into one copied `UX_EVENT_WINDOW_CLOSE`
and deliberately does not destroy the native surface.  NXVM must consume that
event as a distinct product close intent, not as the pause-toggle event:
running closes pause at the next safe boundary and then remove the Window;
paused sessions merely remove the Window; resume recreates the requested
Window surface.  No Window close may stop or resume a session implicitly.

## S3 Contract

SoftPC's monitor reports `Machine paused.` and `Machine stopped.` only after
the corresponding runtime transition completes.  NXVM must establish the same
one-way product route: session runner acknowledges the lifecycle state;
NXVM's product Console retains and formats the resulting completion.  `src/lib`
remains unaware of session state and no runner writes host text directly.  The
product queue is bounded and its overflow is an explicit Console failure, not
a silently dropped completion.

A Console-mode pause releases the borrowed native Console so the NXVM command
Console can report the state and accept `RESUME`, but it must remain a paused
session.  Reclaiming Console on resume is presentation work, not a new session
or reset.  Window close retains the S2 contract and is one producer of the
same paused completion.

A Console pause ends only the synchronous presenter runner after the lease is
released.  It does not emit `stopped`; the later `STOP` command owns that
completion when no runner remains.

### P2 correction

Owner native use rejected P1: lifecycle text was absent although the machine
transition itself completed. The product Console queue and formatter were not
the missing route. The composition adapter incorrectly passed a lifecycle
reporter as a session-manager operation, so no session received its reporter.
P2 installs the reporter through the same selected-session setter used by the
other all-session operations and adds a real-adapter regression. It does not
add a second Console output path or let a runner write host text.

### Owner-approved canonical-refresh amendment

The owner admitted canonical SoftPC revision
`7cc408ec2e27c8243ae2a3f719d313bb7e851e1d` into S3 after NXVM-governance
audit.  Replace `src/lib` byte-for-byte, including its public
`*_interface.h` boundary; retain no compatibility header or local lib patch.

For NXVM's multi-session product, use SoftPC's separation without copying its
single-runtime policy: each composition session owns execution and actual
lifecycle facts, while one VM-product control owner receives copied command,
UX, lifecycle, frame and delivery-failure facts through a bounded FIFO.  It
tags each asynchronous producer with session identity and run generation,
rejects stale input, derives presentation actions, and is the sole Console
formatter.  A session-local presentation binding applies those actions through
the canonical lib and returns completion facts to the product FIFO.  Core
continues to own guest state and snapshots only.

## Exit Boundary

S1 closes only after the complete repository-only unit suite passes.  T526
closure additionally requires its S2/S3 binding sweep, external integration,
canonical manifest and boundary gates, and fresh stripped x64/x86 0526
artifacts.

### Owner-approved product-control migration amendment

The owner further required NXVM to adopt SoftPC's lifecycle and product
experience separation, while retaining NXVM's multi-session capability.  The
next S therefore replaces, rather than wraps, the mixed `vm/platform` UX
route: sessions publish copied facts, process-owned product control owns
identity/generation-aware decisions, and per-session product presentation owns
the only lib leaf.  Core and the shared library remain outside product policy.

### Owner-approved single-session amendment

Before that migration, NXVM product converges to one runtime `vm_session`.
This is a VM/product simplification, not a Core restriction: Core remains a
cardinality-neutral machine library. One product control FIFO will therefore
serve one run generation rather than a matrix of session queues and a second
manager queue. The dynamic product-session manager, selected-session state and
`SESSION LIST/OPEN/SELECT/CLOSE` surface are deleted rather than capped at one.

### S4 result

NXVM product now owns one direct `vm_session` slot. The obsolete Core product
session manager, selected-session state, multi-session command grammar and
their tests are removed rather than retained behind a maximum-one wrapper.
The product Console remains the only lifecycle formatter; it receives reset,
started, paused, resumed and stopped facts only after the composition session
has reached them. The static single-session and readiness gates pass, as do
the complete 305-case repository-only unit suite and owner native UX test.

### S5 result

The product now has one bounded `vm_product_control` FIFO for monitor lines,
lifecycle facts and host input, plus one latest-frame slot which cannot starve
those facts. `vm/product/presentation` is the sole lib Console/Window leaf;
it owns target changes, titles, capture policy and copied Core-frame conversion.
Composition owns execution and emits copied lifecycle/display facts only.
The old `vm/platform` tree, run handle, host-action path and CMake injection
knob are deleted rather than forwarded.

Input is generation-filtered at the product boundary. A retiring lib input
source clears only the keys delivered from that source, emitting their release
while the guest is running; it never leaves a held guest key across a surface
switch or run boundary. The product conversion regression proves the final
80x25 cell, and control regressions prove fact priority, stale input/frame
rejection and retirement cleanup. Focused product tests and the complete
304-case repository-only x64 unit suite pass.

### S6 result

The complete 304-case repository-only unit suite and all 42 registered
external-ROM/media integration cases completed. The initially long Model-40
matrix row was rerun in the clean aggregate and reached its installer terminal;
the IBM 5170 rows also reached their terminals. The full static closure set
now records the actual architecture: the three independent lib CTest routes
are part of the sole T344 registration partition, the migrated
`vm/product/session_factory.c` is the product-to-composition construction
boundary, and NXVM's type/header gates do not impose its private facade on the
manifest-verified standalone `src/lib` corpus and its neutral consumers.

Fresh stripped Release artifacts are `nxvm_0_5_0526_x64.exe`
`01158AD5CBC081DBAB194CC0C0CDF73CA9F9D08B3B325BE3612417D2A5CDD54C`
and `nxvm_0_5_0526_x86.exe`
`1AB7DFC26B8D85771255BCAB5199EA53BF7FD78A486091521C12FDB93F9B49E0`.

### S7 contract

Remove `core/utils/wait.*` and every debugger `Sleep(10)` polling loop. Core's
debugger must not know a host wait primitive. Its target instead receives one
neutral, cancellable completion-wait operation; composition implements that
operation by waiting for the runner's actual pause-or-stop completion event.
The runner remains the sole owner of execution and signals completion only
after its state transition. The debugger must neither busy-wait nor advance the
machine, and no Windows/Linux API may enter Core. Cover `GO` and trace waits,
timeout/failure, reset/stop and teardown so no waiter can outlive its session.

### S7 implementation result

The retired sleep callback, its dedicated Core utility target and its test are
deleted. A debugger target now has exactly one neutral completion operation.
Composition owns two manual-reset events: `completion_ready` is signalled only
after the runner has acknowledged pause or completed exit, while
`control_changed` wakes a paused runner for resume or stop. Thus `GO` and all
trace paths block on a completed execution transition rather than polling a
host delay; Core contains neither a host event nor a platform include.

The completion-target regression executes `GO` twice through the existing
debugger input harness and verifies the two corresponding resume/completion
calls. Focused debugger/pause integrations pass, as does the complete 303-case
repository-only unit suite at eight-way parallelism.

Fresh stripped Release artifacts are `nxvm_0_5_0526_x64.exe`
`1AAE79A2E84472FBE8969B80E110217FF9942D173D96BEDA462A6C8672D10263`
and `nxvm_0_5_0526_x86.exe`
`34582710EC4C3495F19E94270CA0AB2433ED0A0030D6CEA9C58ED153177AEE62`.

### S7 acceptance

The S7 source/test change is 114 added and 155 removed lines (net -41). It
removes the sole Core debugger sleep wrapper and its dedicated smoke rather
than preserving a forwarding compatibility route. The retained execution path
is `Core debugger -> neutral target completion callback -> VM composition
completion event`; the runner alone signals that event after pause
acknowledgement or final exit. The remaining `Sleep(10)` references are
external integration-harness polling, not Core or debugger production paths.

Verification passed: focused debugger regressions, 303/303 repository-only
unit cases, the canonical manifest and Core/debugger/composition/documentation
boundary gates, and `git diff --check`. All 42 x64 Release external integration
cases passed in the same configuration: 22 non-matrix cases together and the
20 serial profile-boot rows in bounded individual invocations required by their
registered shared-host constraint. Fresh optimized stripped x64/x86 0526
artifacts and their SHA-256 identities are recorded above. This accepts S7;
T526 remains open for S8's canonical refresh.

### S8 implementation result (active)

NXVM has adopted SoftPC revision `291afe48a0eefb703569a5cc090b7fd54bf81388`
as an exact `src/lib` corpus. The normalized `types`, `console`, `host`,
`storage`, `ui-base`, `ui-console`, and `ui-window` public roots replaced the
retired `base`/`ux-*` contract in every NXVM source, test and CMake consumer;
no compatibility facade or local library patch remains.

The post-import sweep deleted static gates that opened deleted old routes,
recorded the renamed `ui-base` process-wide source-identity owner, narrowed
the debugger rule to its four real composition adapters, and made the
direct-compilation ledger verify exact classified entries instead of stale
counts. The corpus is byte-identical to the named SoftPC revision; Debug and
Release manifest checks, all current specialized gates and 303/303 Debug unit
tests pass. T526 remains active for its task-level verification and native
review.

### S9 implementation result

`core/debug`, `core/machine`, and `core/product` are now independent Core
modules. The former product debugger, its opaque target contract, xasm and
debugger-local bounded text helpers moved to `core/debug` under `core_debug_*`
names. `core/product` retains only the independent decimal KiB configuration
parser. The sole VM composition adapter binds the neutral debug target to the
machine-debug operations; no Core module imports a peer or a host/VM/session
type.

The old product-debug directory, utility facade, symbols, target names and
tests are deleted rather than preserved as aliases. The renamed boundary gate
checks every Core module's source imports and target edges, while the existing
debugger and VM adapter regressions cover the retained route. Focused 6/6,
repository-only unit 303/303, all current specialized gates, documentation
governance and `git diff --check` pass. T526 remains open for the separately
approved VM event, machine, session and presentation convergence subtasks.
