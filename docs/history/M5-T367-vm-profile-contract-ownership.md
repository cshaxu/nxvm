# M5 T367: VM Profile-Contract Ownership Migration

## Task Record

T367 follows the T366 closure transfer and prepares the profile ownership
boundary needed before later baseline-machine work.  It does not itself close
any CPU, bus, device, phase, or machine-L3 receiver.

## Active Progress

### S1: Ownership inventory and bounded migration seam

S1 inventories every current CPU-profile selector and consumer in core, VM
profile/composition, product tests and the build graph.  Its
[inventory evidence](../etc/evidence/t367-s1-vm-profile-contract-inventory.md)
finds concrete PC/AT and Model 339 selection already in VM profile code; core
retains only generic CPU capability, decoder/execution and timing behavior.
The one boundary requiring migration is the VM session's raw `--cpu` override
into the materialized core configuration.  S2 must turn that into an explicit
VM contract binding while preserving one shared executor and transaction path.
