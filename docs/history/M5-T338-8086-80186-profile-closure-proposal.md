# 8086 And 80186 Profile Closure

## Objective

Close the audit-assigned 8086 baseline and 80186 extension forms, including
their precise profile acceptance, real-mode behavior, prefix/LOCK treatment,
defined FLAGS and stack quirks, faults, and restart ordering.

## Dependency and scope

This candidate consumes the form ledger and shared delivery contracts. It
covers only entries whose minimum profile is 8086 or 80186; later protected-mode
and 32-bit semantics remain in their dedicated candidates. It does not infer
low-profile completeness because a 80386 route happens to execute an opcode.

This candidate follows the shared-delivery package. Its 8086/80186 result is
the legacy baseline consumed by the 80286 and 80386DX candidates, not a
best-effort real-mode subset.

## Completion standard

Each assigned form is implemented and regression-proved in its valid profiles,
or is rejected with its Intel-defined classification and nonpublication
contract. Residual system-state dependencies transfer explicitly.

## T336 Audit-Derived Breakdown

The implementation breakdown starts from a primary-form ledger rather than
from DOS demand:

1. baseline 8086 data, arithmetic, FLAGS, condition/control, stack, string,
   I/O, prefix, and fault matrices, including defined historical quirks;
2. an explicit 8086/80186 `LOCK` legality matrix for every allocated
   memory-capable form: legal atomic memory forms, illegal register/unsupported
   forms, profile gates, prefix ordering, and fault/nonpublication behavior;
3. 80186-only primary extensions (`60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9`)
   with acceptance/rejection, width, and state-publication proof; and
4. a profile-close audit that demonstrates every inherited form has the same
   deliberate 8086/80186 disposition, rather than inheriting an unreviewed
   80386 implementation path.

The detailed form allocation is created at admission from T336's ledger. A
later protected-state requirement transfers to its owning candidate instead of
forcing a duplicate real-mode helper.

## Admission and exit constraints

Admit bounded mechanism slices in dependency order: common decode/prefix or
stack construction first when it owns multiple forms, then form families. Each
slice audits callers, writes, validation, commit, and fault ordering before a
local handler repair. The candidate closes only after its ledger, including
the `LOCK` matrix, has no in-scope partial or unclassified 8086/80186 row.
