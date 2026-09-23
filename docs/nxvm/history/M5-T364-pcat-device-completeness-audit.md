# M5 T364: PC/AT Device, Chip, Port, And Bus Completeness Audit

## Task Record

T364 is the ordered hardware-completeness audit following T363. It creates the
selected-PC/AT inventory required before NMI-source ownership, bus availability
and device-service timing can be admitted. It does not allocate timing or
implement a device.

## Active Progress

### S1: Selected component, route, lifecycle, and timing-owner inventory

S1 inventories all selected core-owned components and optional-interface
absence against retained primary-source provenance and T346--T354 evidence.

S1 is accepted at `df97131a`. Its
[completeness ledger](../etc/evidence/t364-s1-pcat-device-completeness-ledger.md)
classifies every selected component and names the NMI, bus/service and
optional-interface receivers. S2 may only perform the task-level closure
audit; it must not add a device or timing value.

### S2: Task-level completeness closure audit

S2 is accepted at `7d574ae3` and closes T364. Its
[closure audit](../etc/evidence/t364-s2-pcat-device-completeness-closure-audit.md)
confirms complete selected-PC/AT coverage and truthful optional, NMI,
bus/service and physical receivers. No device or timing value was allocated.
