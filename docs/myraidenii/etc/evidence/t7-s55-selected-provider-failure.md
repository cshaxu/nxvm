# T7 S55 — selected-provider failure observation

## Decision

S53 ended at a finite Core fault that could not be assigned to an existing
MyRaidenII owner without exposing prohibited guest fields.  This S adds one
optional trailing observer to the existing X86 Core memory-route descriptor.
Core alone invokes it after an already selected provider returns a terminal
query, read or write status.  `LIB_STATUS_UNSUPPORTED` remains the existing
decline/fallback result and is never reported.

The observer is suppressed for the Core's speculative cache-line eligibility
query: that query deliberately turns any route error into "do not fill", while
the subsequent real bus access remains the sole terminal result.

## Consumers and output boundary

MyRaidenII Board routes retain only `valid`, access direction and `lib_status`.
COP routes retain a separate aggregate with the same three fields, preserving
the existing detailed COP diagnostic's first-failure behavior.  The selected-
set probe emits only those aggregates; it does not format addresses, values,
registers, instruction bytes, or protected media data.

## Proof

The X86 Core memory-device registration smoke proves one notification each for
terminal query, read and write statuses, and proves an `UNSUPPORTED` decline
does not notify.  The existing Board smoke proves Board fallback capture and
COP-route capture/reset behavior through the actual Core route installation.

Focused x64 and x86 builds and both focused executables passed.  Complete
MyRaidenII unit executable sets (11 per width) passed.  The complete X86 Core
suite was rebuilt and executed on both widths (183 executables per width)
after the interface migration.

The registered bounded owner-ROM wrapper then ran once per width with the
existing Coin/Start sequence and containment budget.  Both widths reached the
same finite terminal result.  The safe classification was a selected COP
provider terminal; the Board aggregate stayed clear.  S56 refines the retained
logical range role without relying on this record for an access direction or a
raw guest discriminator.  The wrapper-created external staging child was empty
after each run.  No raw guest fields or probe transcript are retained here.

## Boundary

This is a routing-observation contract, not a new bus, retry, fallback or
runtime repair.  It does not identify a production defect, claim game boot, or
alter selected-set ROM/media ownership.
