# M5 T373: Baseline-Machine And Supported-Device Capability Ledger

## Task Record

T373 freezes the selected IBM PC/AT 5170 Model 339/Type 3, Compaq DeskPro 386
Model 40, and IBM PC/XT 5160-268 capability boundaries and reconciles them
with the current product support surface. It is an audit and allocation task:
it may classify and transfer a capability, but it does not repair a device,
assign a timing scalar, or make an L3-ready claim.

## Active Progress

### S1: current product-support and evidence inventory

S1 inventories actual public/configurable device paths, tests, existing
evidence, and historical machine assumptions. It produces the reproducible
repository-side input to later per-machine primary-source qualification and
functional/timing allocation.

Its [support inventory](../etc/evidence/t373-s1-current-support-inventory.md)
distinguishes the generic 80386/EGA/ATA-capable PC/AT product descriptor from
the Model-339 descriptor, and identifies unimplemented peripheral families and
absent DeskPro/XT profiles without making a completeness or L3 claim.
