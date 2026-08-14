# M5 T361: Legacy Dynamic Arithmetic Timing Authority

## Task Record

T361 consumes the T360 S3 transfer for range-only 8086 and 80186 arithmetic
timing. It must establish a legitimate source formula or an owner-approved
hardware-observation contract before any dynamic timing allocation. It does
not borrow 80386 behavior, choose a range endpoint, or turn host measurement
or another emulator into authority.

## Accepted Progress

### S1: Primary-source formula audit

S1 is accepted at `e8c662f7`. Its [authority audit](../etc/evidence/t361-s1-legacy-dynamic-arithmetic-authority-audit.md) confirms that all transferred 8086 and 80186 Group-3 and immediate-IMUL forms remain table ranges without a primary operand-to-clock formula. The profile-local source routes therefore retain their explicit unallocated successful-retirement transfer, with no endpoint, later-profile formula, emulator, benchmark, or hidden timing allocation.

The only lawful next path is an owner decision: retain the transfer pending new
primary material, or approve the bounded real-hardware observation contract
specified by S1. Focused legacy timing smokes and 245/245 current-gate tests
passed; S1 makes no runtime, ABI, artifact, L3, or cycle-exact change.
