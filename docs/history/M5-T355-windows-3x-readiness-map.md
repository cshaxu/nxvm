# M5 T355: Windows 3.x Readiness Map

## Task Record

T355 consumes the accepted four-profile CPU program and selected PC/AT L3
closure as evidence inputs.  It is a product-readiness dependency map, not a
Windows compatibility or installation task.  It may allocate an evidenced core
device defect to a bounded later subtask, but it may not use guest progress as
the specification for CPU, device, or timing behavior.

## Accepted Progress

S1 was accepted at `8568f9e4`.  Its
[readiness ledger](../etc/evidence/t355-s1-readiness-ledger.md) classifies all
four retained Windows probe sources as non-current, host-observation tools and
allocates a reproducible checkpoint/provenance harness before any guest run.
It confirms that FDC/ATA pending readiness is an accepted baseline, not a
stale implementation blocker, and leaves a future actual checkpoint to S3.

## S2 delivery pending acceptance

S2 adds the explicit, opt-in
[`run-windows31-hdd-checkpoint`](../etc/evidence/t355-s2-opt-in-checkpoint-harness.md)
entry point for the retained HDD/INT13 probe.  It is intentionally outside the
current gate, rejects absent configuration before media access, and leaves the
first actual owner-supplied checkpoint to S3.
