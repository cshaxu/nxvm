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
