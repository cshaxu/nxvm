# M5 Core CPU-To-Board Transaction And Arbitration Contracts

## Purpose

Implement the neutral transaction lifecycle that governs CPU fetch/data/I/O
requests, availability waits, BUSRDY, HOLD/HLDA, DMA, refresh and retirement.
It makes the existing transaction and bus ledgers selectable Core contracts
without placing board or profile names in the CPU executor.

## Shared Admission Baseline

This candidate implements the transaction/arbitration portion of the
[Core specification-driven timing design](../etc/architecture/specification-driven-l3-timing.md)
and consumes the earliest transaction batch from the
[T433 S6 Core L3 admission ledger](../etc/evidence/t433-s6-core-l3-admission-feasibility-ledger.md):
`CPU-PREFETCH`, `TXN-MEMORY`, `TXN-PORT`, `TXN-ARBITRATION`, and
`MEM-RAM-A20-PARITY`. It owns neutral availability, completion and arbitration
contracts; a profile later selects only registered data/contract IDs and may
not add board-specific executor callbacks.

The [T433 S7 source-sufficiency ledger](../etc/evidence/t433-s7-core-source-sufficiency-ledger.md)
is mandatory: selected board READY/HOLD/DRAM inputs are VM facts, while missing
Core lifecycle rules remain blocked rather than being inferred from them.

## Required Scope

Consume T354/T369 transaction evidence, the reconciled route ledger, and the
completed shared locality/BUSRDY mechanisms. Define classification, begin,
wait, grant, completion, cancellation, reset and retirement ownership for each
frozen transaction family. A CPU instruction timing program may request an
external transaction but may not decide a board-specific completion itself.

Use documented constants/formulas/ranges where available; otherwise retain a
labelled deterministic L2 contract. Preserve a single virtual-time owner and
trace observability.

## Module Boundaries And Data Flow

This task establishes the common contract that CPU execution and later
controller/device phase work share.  It does not merge those owners.

```text
VM resolved profile -> copied validated Core plan -> CPU executor
                                                  -> transaction owner
                                                  <-> arbitration owner
                                                  <-> declared device routes
                                                  -> retirement / observation
```

- The CPU executor may request a fetch, memory, or port transaction and may
  consume its typed result.  It never decides board-specific availability,
  completion, grant priority, or a controller's phase transition.
- The transaction owner is the sole owner of transaction classification,
  begin, wait, commit, cancellation and reset.  CPU fetch, CPU memory, CPU
  port, DMA memory and DMA port access use this one lifecycle; a stopped
  debugger API remains an explicitly immediate bounded operation, not a guest
  transaction.
- The arbitration owner is the sole owner of request eligibility, priority,
  grant, release and reset cancellation for CPU, DMA and refresh.  A grant
  cannot expose a partially committed competing transaction.
- A controller/device phase owner owns only its chip state and the production
  or consumption of declared routes (such as DRQ, IRQ, HOLD or availability).
  It cannot write CPU state, commit another owner's transaction, publish a
  second time result, or create a private memory/port path.  The following
  controller/device task supplies those phase owners.
- Core owns the virtual timeline, transaction/arbitration lifecycle and
  observation publication.  VM owns machine identity, source provenance and
  selection of validated data values.  VM may not supply a timing algorithm or
  callback to Core; Core may not branch on a machine/profile name.
- Routes carry only declared typed requests, availability, grants, releases,
  completion or lifecycle outcomes.  Mutable internal CPU, RAM, port, device,
  executor and session state never crosses this boundary.

The required lifecycle vocabulary is `classify -> begin -> wait -> grant when
needed -> commit -> retire`, with the terminal alternatives `cancel` and
`reset`.  A rule with missing selected board evidence is an explicit L2
fallback or plan rejection.  It is never silently converted to a generic
wait-state value.

Any new contract must migrate or remove an equivalent direct or parallel
production path in its changed mechanism.  A forwarding wrapper or a second
state mirror is not a valid completion: the shared owner must become the only
writer and publisher for that fact.

## Ordered Subtasks

| Subtask | Bounded result | Completion boundary |
| --- | --- | --- |
| S1 | Freeze the five transaction capability batches, existing routes, owners, direct-path residues and default behavior in a finite ledger. | No runtime semantic change; every observed route has one intended owner or a named transfer. |
| S2 | Extend the existing copied Core plan only as needed for registered transaction/arbitration selections, explicit L2 exceptions and validation. | No profile callback, profile name or caller-lifetime dependency reaches Core; unregistered/incomplete selections reject before machine publication. |
| S3 | Move CPU fetch/memory/port and DMA memory/port access onto one classified transaction lifecycle. | Success, wait, provider failure, cancellation and reset have one commit/rollback owner; stopped debugger access remains outside guest transactions. |
| S4 | Connect availability, BUSRDY and prefetch selection to that lifecycle. | Fetch fill/consume/flush, wait, HOLD pause and reset are observable; no unsourced board wait value is invented. |
| S5 | Connect CPU/DMA/refresh arbitration and RAM/ROM/A20/parity access classification. | Competition, grant/release, cancellation and reset cannot partially commit or double-charge an access; unsupported latency remains explicit L2. |
| S6 | Connect the sole CPU execution/retirement path, observation and regressions, then audit the ledger against actual routes. | Existing 5170/DeskPro shared routes retain behavior absent an explicit selected contract; trace shows rule/result ordering and reset leaves no stale transaction, grant or observation state. |

S4 and S5 share the S3 owner and may be analysed in parallel, but their code
changes are sequential so that each accepted S leaves one reviewable lifecycle
state.  Controller command, DRQ/IRQ, EOI, media and display phase semantics
remain outside this task and are admitted only by the following controller and
device phase candidate.

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
