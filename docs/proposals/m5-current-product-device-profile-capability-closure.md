# M5 Current-Product Device Profile And Functional-Capability Closure

## Purpose

Freeze the entire publicly supported NXVM device surface, give every retained
capability one valid profile contract and functional owner, and close its
register/state, reset and service completeness before the IBM PC/XT
5160-268 program begins. This candidate follows the DeskPro 386 Model 40 L3
closure audit and precedes the 8088/XT candidates. It is the architectural and
functional prerequisite for, but is not, the later current-product L3 timing
closure.

## Required scope

Begin from the accepted S20 product mechanism for YAML backbone/variant
selection and external ROM-manifest validation with in-memory snapshots. Audit
that mechanism against every retained public profile. If any required generic
catalog, selection, snapshot or constraint-validation behavior is absent,
complete it in `src/vm/product` before migrating device contracts; do not make a
machine-local workaround or assume the first 5170/DeskPro contracts exhausted
its reusable scope.

Then create one source-backed support ledger for every device, controller,
adapter, media path and selectable variant currently public in the product.
Each entry must be retained under one explicit profile contract, marked
unavailable, or removed from the public surface; an accidentally reachable or
compatibility-only path is not a disposition.

Apply the following composition boundary to each retained entry:

- **Core** owns reusable chips, controllers, bus mechanisms and generic device
  semantics, including their common register/state, IRQ/DRQ, reset, error and
  transaction behavior. It never selects an IBM, Compaq, XT, or other machine
  identity.
- **VM device profiles** own the named, source-backed capability contract for
  optional or variant hardware: for example 1.2 MB and 1.44 MB FDDs, a
  specific EGA personality, or an MFM controller. They bind only the reusable
  Core mechanisms required by that contract.
- **Machine backbones/profiles** own the historical combination: fitted
  devices, permitted variants, BIOS and board wiring, machine-local limits,
  and machine-local timing responsibility.
- **YAML** selects only a defined backbone, its permitted variants and media.
  It must reject arbitrary chip composition and any variant the selected
  backbone does not permit.

For every retained public capability, repair or explicitly transfer the
functional gaps in its register set, state machine, reset, IRQ/DMA route,
availability/error/recovery behavior, media or host-copy boundary, and
project-owned consumer proof. Move a shared repair to its earliest Core owner;
retain real hardware combinations, restrictions and board timing in VM. The
1.2 MB/1.44 MB floppy contract is the first instance of this model, not a
special case. Apply the same boundary to DMA/PIC/PIT, RTC/CMOS, display,
input, storage controllers and every other retained public device.

## Dependencies and verification

Admission requires the DeskPro Model 40 L3 audit and S20's YAML
backbone/variant/ROM-manifest snapshot mechanism to be accepted, an inventory
of all public device and YAML surfaces, and the source/provenance boundary for
each declared machine or personality. The audit must explicitly decide whether
S20's generic product mechanism is sufficient for every retained public
profile; any shortfall is in scope for repair in `src/vm/product` before
profile migration proceeds. Before closure require the support and ownership
ledger, profile-contract and backbone/variant validation tests, functional
register/state plus IRQ/DRQ/reset evidence for every retained capability, a
shared-owner sweep, the current gate, and an explicit retained/removed
disposition for every inventory entry.

The following 8088/XT program may use these shared implementations, but still
must audit the 5160-268's own selected board, bindings and timing. The later
current-product device-capability L3 candidate independently closes
profile-local service timing and event ordering for every retained public
capability.

## Non-goals and stop conditions

Do not invent a free-form machine builder, broaden the public hardware surface,
claim board or physical timing, use BIOS/ROM/media bytes as committed inputs,
or treat one machine's personality as proof for another. Stop an unsupported
or unsourceable capability at a truthful retained/removed decision; do not
leave it publicly selectable merely because a generic Core mechanism exists.