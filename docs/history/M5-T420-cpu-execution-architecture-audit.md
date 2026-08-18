# M5 T420: CPU Execution Architecture Audit

## Withdrawal

T420 was admitted to re-establish a four-profile instruction/state/timing
inventory. Immediate source review proved this work already exists: T401 S1-S78
contains the complete frozen decoder/form requirement matrix and independent
closure audit, while T360 S1-S5 reconciles every selected timing source and
consumer. Repeating that work would create a competing audit path rather than
new evidence.

T420 therefore makes no executable change and closes as a duplicate admission.
The authoritative retained CPU audit is T401, with T360 for timing. Its only
remaining CPU boundaries are the specifically named TODO receivers such as
source-formula gaps, numerical x87 execution, VME/PVI, and product/physical
timing; none is silently accepted or reopened here.