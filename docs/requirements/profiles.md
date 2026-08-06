# Profile Requirements

Profiles describe a selected VM machine shape or VDM execution capability set.
They do not compose a product, create a machine, manage threads, open media,
or access a sibling module directly.

## VM Profiles

A VM profile declares topology, enabled devices, boot policy, ROM assets,
default CMOS data, firmware-provider identity, and acceptance expectations.
Its identifier uses `nxvm.machine.<name>`. New profiles, including Compaq
Deskpro 386 or IBM PC110, require their own design gate, legal review, fixture
policy, and regression plan.

## VDM Profiles

A VDM profile declares the owned DOS capability set: memory and service policy,
optional firmware-service subset, device policy, and required host
capabilities. Its identifier uses `nxvdm.execution.<name>`. It never implies
POST, a boot ROM chain, or a whole-machine topology.

## ROM And Firmware Overrides

ROM bytes, fonts, mapping locations, reset metadata, and provider selection are
profile data. A profile-specific BIOS or firmware implementation is allowed
only as an override provider against a public core callback contract. It may
fill a gap in generic capability, but may not create a machine, call platform,
choose a product, or manage a thread. Root composition binds its callbacks and
owns its lifetime.

External ROM policy, redistribution limits, and user-supplied bundle rules are
defined by `source-policy.md` and the future profile design gate.
