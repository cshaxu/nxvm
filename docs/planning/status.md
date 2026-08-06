# Project Status

## Current Work

**M5 T251 S2 active: implement bounded cancellable lifecycle waits.**

Original request: execute the third admitted T248 migration without changing
guest time or NXVM lifecycle behavior. S1 fixed the host-only cancellation
contract and classified every existing wait site. S2 now implements the neutral
primitive and adopts it only at lifecycle waits with existing cancellation
facts. Raw host-clock observation remains deferred.

| Requirement | S2 evidence |
| --- | --- |
| Primitive | Add one bounded core/platform completed/cancelled wait with no guest-time or raw-clock exposure. |
| Adoption | Convert composition and VM lifecycle waits only; retain `core/utils` debugger wait scopes. |
| Preserve products | No new thread, watchdog, guest mutation, product policy, or lifecycle path. |

Applicable rules: module layout, contracts, coding/source policy, execution
workflow, and one active subtask. Planned commands:
`rg -n "core_platform_sleep|core_platform_wait|core_utils_wait|wait_for_flip|wait_for_pause" src/core src/vm tests`
and focused wait/run-handle smoke. Expected marker:
`M5:T251:S2:CANCELLABLE-WAIT-MIGRATED:OK`. Stop for owner direction if a site
requires guest time, raw host clock, unbounded waiting, display/exit policy,
or a second lifecycle path.

T248 is closed: it established the evidence-backed T249--T252 migration queue
and deferred raw host-clock observation. T249 and T250 are closed with
artifacts `0.5.0248` and `0.5.0249`; T251 now starts with its own S1 contract
in [its task record](m5-t251-cancellable-host-wait.md). Existing unstarted
hardware work begins at T253; completed task identities and artifact revisions
do not change.

## Current Technical Baseline

- **T250 S3:** `current-gcc` and `verify-current-artifact-target` select
  `vm-0-5-0249`; static/ownership checks and 85/85 CTest cases passed.
  Artifact `nxvm_0_5_0249.exe` SHA-256:
  `E6EA5C35F1C6EDFD130E3BCCC5A6EBD1BF92442B85E715505E4B3FF5CCF34079`.
- **T243--T246:** core owns checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans. T247
  verifies the current artifact target and full gate over that boundary.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

Completed implementation detail, artifact history, and rationale are in
[M5 History](../history/m5.md) and Git history. The active task packet above,
the roadmap, and the M5 closure checklist are the only current operational
authorities.

## Recent Governance Updates

- **M5 Td S20:** aligned the five-component architecture.
- **M5 Td S21:** added source-distribution notices and release-record gates.
- **M5 Td S22:** aligned roadmap and trusted external-research boundaries.
- **M5 Td S23:** compacted completed M5 records into history, corrected the
  historical baseline, clarified notice provenance, and removed an untracked
  temporary error file. This documentation task changes no runtime behavior.
- **M5 Td S24:** fixed the time/device ownership vocabulary: core owns guest
  ticks and generic PIT/PIC/DMA mechanics; VM owns PC/AT CMOS/RTC and BIOS time
  semantics; composition owns the bounded product pump and host pacing. This
  documentation task changes no runtime behavior or active-task scope.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Roadmap](roadmap.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
