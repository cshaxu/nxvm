# M5 T373: Baseline-Machine And Supported-Device Capability Ledger

## Task Record

T373 freezes the selected IBM PC/AT 5170 Model 339/Type 3, Compaq DeskPro 386
Model 40, and IBM PC/XT 5160-268 capability boundaries and reconciles them
with the current product support surface. It is an audit and allocation task:
it may classify and transfer a capability, but it does not repair a device,
assign a timing scalar, or make an L3-ready claim.

## Accepted Work

### S1: current product-support and evidence inventory

S1 inventories actual public/configurable device paths, tests, existing
evidence, and historical machine assumptions. It produces the reproducible
repository-side input to later per-machine primary-source qualification and
functional/timing allocation.

Its [support inventory](../etc/evidence/t373-s1-current-support-inventory.md)
distinguishes the generic 80386/EGA/ATA-capable PC/AT product descriptor from
the Model-339 descriptor, and identifies unimplemented peripheral families and
absent DeskPro/XT profiles without making a completeness or L3 claim.

### S2: primary-source BOM qualification

S2's [BOM and source ledger](../etc/evidence/t373-s2-baseline-bom-source-ledger.md)
records only qualified machine identity, CPU, memory, firmware, input, storage
and display facts for Model 339, DeskPro Model 40 and PC/XT 5160-268. It leaves
every board/device field without a machine-specific primary fact unknown and
transfers support-status classification to S3.

### S3: classified capability and ownership ledger

S3's [classified capability ledger](../etc/evidence/t373-s3-baseline-capability-classification.md)
reconciles the S1 implementation inventory with the S2 machine BOM. It proves
that no selected baseline capability is complete, distinguishes 5170's partial
logical foundation from the empty DeskPro/XT compositions, and assigns every
retained generic product capability to the current-product closure rather than
to a baseline by inference. T373 still requires its task-level closure audit.

### S4: task-level closure audit

S4's [closure audit](../etc/evidence/t373-s4-task-closure-audit.md) maps every
proposal requirement to S1--S3 evidence and verifies the exact functional,
timing, final-audit, current-product, or TODO receiver for every remaining
gap. It concludes that T373 may close only as a planning/ownership task: no
baseline has reached model-L3 and the next implementation receiver is the
queued Model-339 selected-device functional closure.
