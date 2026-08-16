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

Begin from S20's actual product mechanism: startup catalog discovery, a frozen
entry, the one fixed Model-40 backbone, and its external two-ROM manifest with
in-memory copies. S20 does **not** supply generic backbone/variant selection,
an allowed-variant table, or a universal media contract. Use the accepted
Model-40 functional matrix and L3 audit, plus the Model-339 contract, as inputs
rather than treating either machine as a generic builder.

First audit the retained public catalog, selection, snapshot and
constraint-validation behavior. Where the inputs demonstrate a shared need,
complete one generic `src/vm/product` contract before profile migration; do
not make a machine-local workaround or assume the first 5170/DeskPro contracts
exhaust its reusable scope. That contract must make each backbone declare its
fixed properties, default configuration, allowed named variants, prohibited
combinations, ROM/firmware manifest requirements, startup-only fixed-media
rules, and removable-media lifecycle. YAML may select only such a defined
backbone, one of its declared allowed variants, and admitted media. It must
reject arbitrary chip composition, forbidden variants, and any fixed-media
replacement after session publication.

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
- **YAML** selects only a defined backbone, its allowed variants and media.
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

Admission requires the DeskPro Model 40 L3 audit, the accepted Model-40
functional matrix, Model-339 current contract, an inventory of all public
device and YAML surfaces, and the source/provenance boundary for each declared
machine or personality. The admission audit must decide which fixed
configuration and allowed-variant facts are actually proven for each backbone;
no desired but unsourced option becomes public merely because a Core mechanism
exists.

Before closure require the support and ownership ledger, profile-contract and
backbone/variant validation tests, startup snapshot and fixed/removable-media
lifecycle tests, functional register/state plus IRQ/DRQ/reset evidence for
every retained capability, a shared-owner sweep, the current gate, and an
explicit retained/removed disposition for every inventory entry.

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
