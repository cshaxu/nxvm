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
S1 is accepted at `115f3f3b`; it changes no runtime behavior and makes no L3
claim.

### S2: VM-owned contract binding

S2 routes the default-PC/AT CPU/FPU option through a VM-owned capability and
timing contract before the existing single core-config materialization path.
The Model 339 retains its descriptor-selected 80286 contract, while generic
direct-core fixtures remain unchanged.  Its
[binding evidence](../etc/evidence/t367-s2-vm-profile-contract-binding.md)
records focused profile/session proof and the unrelated existing full-build
blocker; it makes no timing, bus, device or L3 completion claim.
S2 is accepted at `f60d87ea`; T367 is closed.  The profile ownership migration
does not close any of the independently queued L3 receivers.
