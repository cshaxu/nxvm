# M5 Core CPU-To-Board Transaction And Arbitration Contracts

## Purpose

Implement the neutral transaction lifecycle that governs CPU fetch/data/I/O
requests, availability waits, BUSRDY, HOLD/HLDA, DMA, refresh and retirement.
It makes the existing transaction and bus ledgers selectable Core contracts
without placing board or profile names in the CPU executor.

## Required Scope

Consume T354/T369 transaction evidence, the reconciled route ledger, and the
completed shared locality/BUSRDY mechanisms. Define classification, begin,
wait, grant, completion, cancellation, reset and retirement ownership for each
frozen transaction family. A CPU instruction timing program may request an
external transaction but may not decide a board-specific completion itself.

Use documented constants/formulas/ranges where available; otherwise retain a
labelled deterministic L2 contract. Preserve a single virtual-time owner and
trace observability.

## Dependencies

Consumes the machine-plan boundary and CPU timing programs. It precedes device
phase contracts, which attach producers and consumers to this lifecycle.

## Evidence And Completion Standard

Require a finite transaction-family ledger, focused tests for memory/I/O,
wait/BUSRDY, DMA/HOLD/refresh priority, cancellation/reset and retirement
ordering, plus regressions proving existing CPU and 5170/DeskPro shared routes
do not change without an explicit selected contract.

## Non-goals And Stop Conditions

Do not claim electrical bus cycles, add profile callbacks to the executor,
model undocumented board propagation, or revise device command semantics.
L4 is prohibited. Transfer a board-specific fact with no neutral contract to
the later VM/profile program rather than fabricate timing.
