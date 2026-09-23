# App/Core Design

S3 owner-requested revision: the separate core/host organization in the prior
design is withdrawn. Machine driver, cartridge media, debug, input and frame
adaptation belong under core/machine alongside the NES hardware owners. The
[current receiving proposal](../etc/m2-execution-management-context.md#revised-component-boundary-for-owner-confirmation)
and Source Layout contain the revised component/file tree awaiting confirmation.

## Objective And Boundary

Design the first NES execution/management increment against the adopted Common
driver/session APIs. M1 closure is the prerequisite. This package includes M2
planning, ownership, lifecycle/media/debug contracts and a finite verification
matrix. It creates no source or executable; later candidates require admission.

## Approach And Exit

First settle component ownership and actual shared integration constraints.
Then settle exact public signatures/value layouts, command grammar/state matrix,
CPU/opcode and bus coverage, cartridge limits, error handling and named tests.
Architecture owns component decisions; indexed design detail owns derived API
tables. Complete only when no implementation-critical contract remains implicit.
Stop if a shared ABI change is needed; do not patch the imported roots locally.

## M2 Design Coverage

Single machine, one cartridge; iNES 1 mapper 0 only; reject NES 2.0, trainers,
other mappers and unsupported extensions explicitly. No PPU/APU/controller
emulation, sound, display, cartridge persistence or external game claim.
Use original bounded synthetic CPU programs. The [CPU contract](../etc/cpu-bus-cartridge.md) and its 151-row matrix fix all
documented instructions, flags, addressing, cycle and interrupt proof. The other
105 bytes produce copied guest traps in PAUSED, never successful no-ops.

## Verification

Read actual Common APIs, trace each operation through one executor, review all
failure/ownership boundaries, run documentation governance and tooling self-tests.
Runtime evidence remains the responsibility of subsequently admitted packages.

## Review Delivery

The [complete design ledger](../etc/app-core-verification.md) maps all App/Core
domains to detailed contracts and future acceptance cases, with authoritative
references. The owner requested a stop for interactive verification once this
design is delivered. No implementation candidate is admitted by completing it.
