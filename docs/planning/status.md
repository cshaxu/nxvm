# Project Status

## Current Work

**M5 T248 S3 active: close the admitted host-capability boundary.**

Original request: execute the core/platform versus VM/platform boundary
admission after T246's machine-start boundary. S1 established that copied
normalized input, a copied presentation mailbox, and cancellable host wait
have concrete NXVM and trusted-research needs; raw monotonic-clock observation
is deferred. S2 froze T249--T252 with a smallest-contract and owner rule. S3
now checks that task numbering, queue references, and exclusions have one
meaning before source migration begins.

| Requirement | S3 evidence |
| --- | --- |
| Frozen implementation tasks | T249 input/source ingress, T250 copied presentation mailbox, T251 cancellable wait, and T252 VM composition lifecycle closure each have a narrow contract and stop condition. |
| Exclusions | Raw host monotonic clock, file/path, drive, CLI, window, exit, DOS policy, guest mutation, and VM run-handle/display policy stay outside core/platform. |
| Preserve products | No source/build/runtime change in T248; no host-thread guest mutation, raw guest pointer, or second execution path. |

Applicable rules: module layout, contracts, coding/source policy, execution
workflow, and one active subtask. Planned commands:
`rg -n "host|presentation|input|wait|clock" src/core src/vm src/vdm docs/architecture`
and `git diff --check`. Expected marker:
`M5:T248:S3:BOUNDARY-CLOSURE:OK`. Stop for owner direction if a frozen
contract requires filesystem/path policy, direct guest mutation, raw guest
memory, product display/exit policy, a raw host-clock contract, or a second
queue/run loop.

T248 is an execution task, not a terminal design note: S2 freezes the
conditional T249--T252 migration queue in
[the T248 record](m5-t248-host-capability-admission.md) before any platform
source move. Existing unstarted hardware work begins at T253; completed task
identities and artifact revisions do not change.

## Current Technical Baseline

- **T247 S4:** `current-gcc` and `verify-current-artifact-target` select
  `vm-0-5-0247`; 34 static/ownership checks and 85/85 CTest cases passed.
  Artifact `nxvm_0_5_0247.exe` SHA-256:
  `51D45A24C6C41E3D43D5E53E00CF0C3862FF01250F007D1D33787F09C86FECF4`.
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
