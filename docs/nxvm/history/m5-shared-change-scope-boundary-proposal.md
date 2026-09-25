# M5 Shared Change-Scope Boundary

> Retired superseded proposal, not a candidate or rule. The accepted
> [Execution Policy](../../rules/EXECUTION.md#change-discipline) permits one
> hosting T to deliver separate NXVM, MyNES and Shared commits, with exactly
> one target per P. The text below preserves the rejected earlier design.

## Objective

Make product isolation an execution invariant: an NXVM or MyNES task changes
only its named product; cross-product work is an explicitly admitted Shared
task, never an incidental side effect.

## Rule

- `NXVM` and `MyNES` T/Td work changes only the named product's source, tests,
  assets, tools, configuration and documentation.
- `Lib`, `Common` and `x86` work changes only that one neutral component and
  its matching tests.
- Only an explicitly `Shared` T/Td may change more than one product, root
  shared configuration/governance, or multiple shared components. Its packet
  lists each affected consumer and its verification.

Reading a sibling or shared component is always allowed; changing it is not.
A discovered cross-boundary requirement becomes a separately admitted task.

## Exit Standard

The shared execution rule states this boundary, commit scope names match it,
and both product documentation gates still pass.
