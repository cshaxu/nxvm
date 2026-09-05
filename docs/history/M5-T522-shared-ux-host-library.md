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
