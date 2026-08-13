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

## Completion standard

Each assigned form is implemented and regression-proved in its valid profiles,
or is rejected with its Intel-defined classification and nonpublication
contract. Residual system-state dependencies transfer explicitly.
