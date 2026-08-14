# T353 S2: Selected PC/AT Profile-Topology Contract

## Defect Class And Owner

S1 found that `vm_profile_default_pc_at_port_range` was an overloaded,
contiguous address-envelope model.  It could not express sparse leaves, access
direction, second-DMA/page registers, memory-owned port `92h`, or the fact
that ATA carries port/width/feature information outside that range list.  It
also omitted the selected compatible AUX IRQ12 source because a device-keyed
route record implicitly assumed one route per device.

The owner is the default PC/AT profile descriptor and its VM-composition
consumers, not the core port dispatcher.  S2 replaces the envelope with exact
`vm_profile_default_pc_at_port_leaf` records and named route sources.  Core
continues to own all provider registration, device state, IRQ/DMA requests,
reset, finalization, and deterministic timeline advancement.

## Contract

Each selected profile leaf now has one device role, address, and read/write
claim.  The default descriptor explicitly lists primary DMA command leaves,
all selected PC/AT DMA page leaves, secondary-DMA leaves, KBC `60h/64h`,
memory-control `92h`, sparse VADP CRTC/mode/status leaves, CMOS, sparse FDC,
and ATA command/alternate leaves.  No profile entry claims KBC `61h-63h`, VADP
`3D6h/3D7h`, or FDC `3F3h` as providers.  `3F6h` remains the ATA alternate
status/device-control leaf, not a missing FDC registration.

The FDC `3F7h` claim is deliberately read/write: its direction register is
read while its configuration-control register is written through the same
selected leaf.  This was confirmed against the existing FDC registration path
during S2; the first S1 ledger wording is superseded by this exact contract.

Named routes make the selected producer topology explicit:

| Source | IRQ | DMA |
| --- | --- | --- |
| PIT IRQ0 | 0 | none |
| KBC keyboard | 1 | none |
| KBC compatible AUX | 12 | none |
| CMOS/RTC | 8 | none |
| FDC | 6 | 2 |

ATA retains `hdc_pio` as the single owner for its IRQ14, data/register widths,
LBA28, and channel-presence semantics.  Its port leaves participate in the
same topology check; those semantic fields are not copied into a second route
record.

`vm_profile_default_pc_at_descriptor_is_valid()` compares the descriptor's
complete leaf/route table and ATA semantic descriptor with the one selected
default-PC/AT contract.  `vm_session_storage_initialize()` performs that check
before creating the core machine or registering/configuring session
controllers.  Session display, CMOS, FDC, and ATA configuration then consume
the validated leaves/routes.  This gives malformed profile materialization an
atomic failure boundary: no core machine is published.

## Similar-Issue Sweep

The sweep replaced every `port_ranges`, `port_range_count`, and
`vm_profile_default_pc_at_port_range_find` consumer in `src/` and current
tests.  It also reviewed profile routes, FDC/HDC composition, VADP/CMOS
configuration, base core registration, KBC AUX, DMA, reset/finalize, and all
port assembly/controller evidence.  No other default-profile range consumer
or parallel route table remains.  The only former special representation,
`hdc_pio`, remains intentionally single-source and is checked by the common
descriptor validator.

## Proof

`current.vm-pcat-topology-s2-smoke` creates the ordinary VM session, checks
every profile leaf against the frozen core port registry, confirms selected
sparse non-leaves remain unregistered, validates all five routes, and mutates
a leaf in a cloned descriptor to prove rejection before core-machine
publication.  The retained default-profile and session-initialization
atomicity smokes prove the profile contract and lifecycle consumers.

Focused execution passed all three markers.  Fresh configuration, exact
current-gate registration, documentation governance, diff check, and the full
current gate are recorded with the S2 delivery.  No core port ABI, controller
command behavior, external device, or timing model changed.

Promotion: retain through T353 closure, then merge the final declaration and
lifecycle result into the T353 history record.
