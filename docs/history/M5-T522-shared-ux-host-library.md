# M5 T522: Shared UX And Host Library

T522 extracts the owner-provided shared library as independent `ux`, `host`,
`session`, `storage`, and `observability` components, then converges NXVM on
the retained paths. The proposal defines the intended source layout and
subtask sequence. No library component owns a machine, profile, firmware,
guest state, or another library component.

## S1 Admission

S1 is a bounded corpus, ownership, provenance, and contract audit. It creates
the finite ledger needed to determine which staged and current NXVM mechanisms
may move into each independent library component, which must remain product
local, and which must be excluded. It changes no source or build path.

## S1 Result

The staged 23-file Win32 corpus byte-matches its owner-provided SoftPC source
and carries no independent notice. Its existing frame/event/router/binding
intent is usable, but its action and mailbox APIs leak Win32 types. Direct
relocation is therefore rejected. The accepted S2 scope is only the bounded UX
root/Win32/Linux split; its complete disposition ledger is in
[T522 S1 evidence](../etc/evidence/t522-s1-shared-library-migration-ledger.md).

The proposed `host`, `session`, `storage`, and `observability` roots have no
admitted current implementation. T522 cannot create them as empty framework
directories: each needs a later finite candidate ledger and exact replacement
boundary. This preserves NXVM's Core/VM/session ownership while leaving a
clean reusable target for the three products.

The isolated x64 Debug build completed the full repository-only unit suite:
`317/317` passed (758.00 process seconds). Documentation governance and
`git diff --check` passed. The earlier `coverage-gcc` tree was not used as
evidence because it had a stale registered executable missing before S1.

## S2 Result

S2 creates the admitted self-contained `lib/ux` source surface: root copied
values/actions/router/binding/mailbox, a native Win32 Console/Window runner,
and a native Linux Console source. It deletes the staged duplicate native
mailbox/event queue rather than wrapping them. NXVM has not cut over: the
existing VM platform still owns its production route until S4 can replace and
remove it atomically. The S2 [evidence](../etc/evidence/t522-s2-ux-library-extraction.md)
records the full source manifest, ownership sweep and `306/306` unit result.

## S2 Acceptance

The coordinator independently reviewed the actual `4d96a052` diff and accepts
S2. The root headers are value-only and do not name a native SDK, Core, VM,
machine, profile or session type; native implementations depend only on the
UX root. The process-wide Console lease carries no product state, while the
Window's mutable presentation state is allocated in its per-run context. The
only retained production presenter path is still NXVM's existing path, by S2
design; S4 must replace and delete it atomically. The coordinator re-ran the
complete repository-only unit suite (306/306), documentation governance, and
`git diff --check`. The library source delta is `+2480/-2` tracked lines:
the positive count is the owner-provided 23-file UX corpus newly brought under
version control and reshaped into the neutral contract, not a second NXVM
production route.

## S3 Result

The complete non-UX candidate sweep admits no extraction. Each current
lifecycle, wait, storage and diagnostic path either owns Core/VM semantics or
lacks a complete independent replacement/caller boundary. The [S3 ledger](../etc/evidence/t522-s3-nonux-capability-admission-ledger.md)
records every disposition. The task therefore retains no empty library root
and advances only to the planned S4 one-path UX cutover.

## S3 Acceptance

The coordinator independently reviewed `e11a3f1a`: it changes only the
indexed admission evidence, history and active-status record, and its six
candidate-family dispositions match the referenced source/caller sweep. It
does not hide implementation in a documentation delivery or create a library
root. The complete repository-only unit suite remains 306/306 and
documentation governance passes. S3 is accepted with no transfer other than
the already planned S4 UX receiver.

## S4 Result

S4 replaces NXVM's native presenter/input route atomically. The retained
production flow is one copied Core mailbox entry through one VM conversion to
one `lib/ux` mailbox and native shared Console/Window loops. The VM binding is
the only product action and guest-ingress adapter; `lib/ux` public headers
remain value-only. The old Win32 presenters, Linux Console, host surface,
display-control stubs and VM shortcut classifier are removed. Linux special
key mapping is a small reusable value helper with a repository unit proof.

## S4 Acceptance

The coordinator reviewed `11d0f9e1` against the S1 migration ledger and found
no old route, no public Core/VM/native-type leak, and no parallel presenter.
The cutover removes 1,962 tracked lines overall. Full repository unit
verification is 304/304; focused shared-UX binding/frame/Linux-key tests,
strict Linux-path static analysis, documentation governance and diff checks
pass. The owner accepts source/static/unit Linux verification in place of a
native Linux host run. S5 is limited to product/integration/artifact closure.

## S5 Result And Acceptance

The shared route closes with no legacy presenter, input or mailbox route.  A
large copied UX frame is now conversion scratch owned by the VM platform
context, so display publication cannot exhaust a session thread stack.  Every
registered host action first releases its consumed chord through the neutral
UX binding, then calls the product action callback; pause, mouse release,
Ctrl+Alt+Del and Alt+Enter therefore share one ordering rule.  The product
callback alone decides the requested operation or guest sequence.

The external boot matrix exposed a test scheduling defect: concurrent turbo
boots made Model 40's wall-clock terminal a function of host CPU contention.
The four bounded real-boot probes are now CTest-serial host-resource tests;
ordinary integration cases remain parallel.  This is test scheduling only and
does not change a machine, controller, profile, firmware or media route.

Focused UX contract/binding/frame and mouse-driver tests pass.  Full unit is
304/304.  Complete external-YAML integration is 44/44, with no failure marker.
The portable Linux key-map unit passes; its curses adapter was source-reviewed
but not compiled on a native POSIX target, per the owner-approved no-Linux-run
scope.  `git diff --check` and the legacy-route sweep pass.  The single current
target produced stripped optimized `nxvm_0_5_0522_x64.exe`
(`13ED2D6D69BA012F36E28EBFF580F00F2FC8B5DA61306C458794ED66CC119EED`)
and `nxvm_0_5_0522_x86.exe`
(`EAEC436BBF2AB9776800E260B335D85129D5FC7F59D8EBE80B7AB0802A61DC0B`);
the configured PE checks confirm their architectures.

## S6 Result

The owner rejected the former UX-only task closure: T522 remains responsible
for the five independent shared capabilities stated in its proposal.  The
[complete capability ledger](../etc/evidence/t522-s6-complete-shared-capability-ledger.md)
maps each `host`, `session`, `storage` and `observability` candidate to a
bounded receiver, old-route deletion boundary, retained product/Core policy
and regression owner.  It explicitly prohibits empty library roots and raw
machine/session pointer exposure.

The S6 RDP closure test sends UTF-16 through the existing shared Win32
normalizer and UX binding, observing only VM guest key make/break output.  It
adds no product route.  The focused proof and full repository-only unit suite
pass; S7 begins the host synchronization replacement.  T522 is open.

## S7 Result

`lib/host/sync` now owns the sole generic host sleep, yield and
cancellation-aware wait contract. Its root header remains native-free; the
Win32 and Linux implementations are the only native leaves. The old
`core/platform` sleep/wait implementation and its test were deleted, and every
Core/VM/test caller now consumes `host_sync_*`. The independent host test is
under `test/lib`, links only `host-sync`, and proves completed, cancelled and
invalid waits.

The legacy-symbol sweep is empty. The regenerated test tree runs host sync as
`unit.host-sync-smoke`; complete unit is 304/304 and external-YAML integration
is 44/44. Current 0522 x64 and x86 artifacts have passed their architecture
checks. T522 stays open for session, storage and observability extraction.
