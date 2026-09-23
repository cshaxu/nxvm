# T394 S1 80386 Physical-Qualification Admission Ledger

## Frozen Current Surface

The only current Core retirement contracts are deterministic and physical.
Physical mode rejects a successful retirement only when the shared source-timing
classifier marks it unallocated, before executed count, elapsed ticks and
clock-domain publication. The two Model-40 session construction paths select
deterministic mode. No profile-local classifier, stage callback or mutable VM
state reaches Core.

The accepted M40-C0 corpus remains the only complete 80386 startup stage:
18,255 classified successes, zero unallocated successes, 75 parents and 82
contextual children. T390 S27 maps each child to an exact Intel-primary owner
and focused regression. It is source-qualified evidence, not current physical
eligibility. T390 S34/S35 prove why: the binary Core contract cannot enforce
C0 membership, so a later classified success could publish physical time.

C0A is an immediate post-C0 `IN AL,61h` observation, not a complete stage.
The later C1 protected-entry and FDC observations are bounded non-successes and
do not define a finite 80386 success universe. The residual ledger continues
to retain every other successful 80386 form/context nonphysical.

## Initial Ledger And Decision

| Candidate stage | Universe/proof state | Current enforcement | Disposition |
| --- | --- | --- | --- |
| M40-C0 | Complete 82-child exact-row matrix and direct regressions | No immutable Core stage-membership mechanism | Retain deterministic; candidate for a later shared mechanism design. |
| C0A | One copied post-C0 port-61h read | No complete form/context matrix | Observation only; not eligible. |
| C1 | No finite successful corpus or accepted endpoint | No corpus and no enforcement | Nonphysical; do not replay toward it. |
| All residual 80386 successes | T388 residual partition | Physical contract rejects only unallocated rows | Explicitly nonphysical. |

The next bounded receiver is a shared-Core immutable eligibility-mechanism
design: prove whether a finite, source-qualified stage descriptor can be
validated at machine construction and applied before time publication without
Core depending on VM state or creating a profile-local execution path. It must
include complete-stage membership, reset/failure semantics, an out-of-stage
classified-success nonpublication regression and the two Model-40 construction
caller sweep. If that mechanism cannot be expressed without a wider CPU
contract, it transfers to the earliest Core lifecycle/qualification owner.

No physical mode, board clock, device clock, firmware asset, timing row or L3
claim is selected by this S.