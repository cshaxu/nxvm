# M5 T588: OmniBook 300 hardware, ROM and execution design

Owner admitted this first OB300 T on 2026-11-19 after T587 closure. [Queue](../states/QUEUE.md) owns priority;
[family research and shared boundaries](m5-omnibook-emulator.md) apply.

## Objective and scope

Freeze OB300 plus System ROM 1.0 ABA as one machine/card pair. HP service
manual 5965-0244 identifies a 20 MHz 386SXLV; do not configure the 425/430
486SLC/e or PC110 486SX as an unexplained substitute. Verify silicon-specific
CPU, bus and firmware-visible differences against current X86 Core.

Inventory reset aliases, BIOS/system-card windows, common/attribute memory,
CIS, FFS20 access, RAM, C&T 65510, input and HP control/power registers.
Distinguish measured ROM contents from unproved hardware address decoding.
Check existing masked-span/provider-decline memory-routing debt for relevance.
Define App versus shared ownership against today's packages, with no assumed
Lib/Emulator changes. Establish current cached Release performance baselines
and a measurement method using fixed work and named firmware checkpoints.

## Dependencies

None; this is the first OB300 implementation candidate.

## Non-goals

No complete boot claim, 425/430 implementation or speculative CPU rewrite.

## Completion and stop conditions

A finite convergence ledger maps every required boot/device route to its
owner, evidence grade, existing implementation/gap and receiving task. Include
an executable reset-mapping proof where feasible, an explicit first missing
hardware boundary, and the minimal startup design. Freeze measurable DOS boot
and input-response budgets before implementation qualification; historical
PC110 timings are risk evidence, not OB300 measurements.

At admission, plan bounded numeric S batches from the convergence ledger rather
than one S per symptom. Reuse existing tests where they cover the contract.
Missing manuals alone are not an indefinite blocker: document L1 assumptions,
L2 reference models or L3 explicit manual formulas/constants honestly. Public
interface changes follow the applicable review rules. New evidence that
invalidates a prerequisite requires a recorded design/scope decision, not a
second execution route or an unreported transfer.

## Admitted execution sequence

| S | Bounded batch | Completion evidence |
| --- | --- | --- |
| S1 | ROM identity, current hardware/owner inventory and gaps | Verified archive metadata plus finite convergence ledger; no boot claim. |
| S2 | Reset/card mapping proof and execution baseline | Run the actual reset bytes through the existing executor using explicitly labeled mapping hypotheses; identify the first unsupported route and measure fixed-work costs. |
| S3 | Minimal board design and implementation handoff | Reconcile every ledger row, freeze numeric usability budgets from the measurements and identify required public-contract reviews and receiving tasks. |

[Convergence ledger](../etc/evidence/t588-ob300-convergence.md) is evidence,
not a second queue. S1 is initially admitted; later batches require their
packet before work. No new T is implied for each missing port. If S2 needs an
unapproved public change, give its exact minimal design and evidence before
implementation. This T closes only when its design/proof/budget predicates
are met; it cannot claim the later ROM DOS or Windows tasks completed.
