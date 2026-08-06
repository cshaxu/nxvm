# Project Status

## Current Work

**M5 T249 S1 active: copied input-source and composition-ingress contract.**

Original request: execute the first admitted T248 migration without changing
NXVM input behavior. Define the copied neutral input value and source/sink
contract, then inventory every Console/window producer and composition
consumer before moving source. The current composition ingress remains the
only execution-boundary queue.

| Requirement | S1 evidence |
| --- | --- |
| Contract | One copied core/platform input value plus source/sink call boundary; composition remains sole owner of enqueue/dequeue order. |
| Inventory | Account for Win32 and Linux Console/window producers and their current VM composition consumer. |
| Preserve products | No S1 source/build/runtime change; no host-thread guest mutation, raw guest pointer, new host queue, or altered input UX. |

Applicable rules: module layout, contracts, coding/source policy, execution
workflow, and one active subtask. Planned commands:
`rg -n "keyboard_transport|mouse_transport|receive_.*event|request_transport" src/vm tests`
and `git diff --check`. Expected marker: `M5:T249:S1:INPUT-CONTRACT:OK`.
Stop for owner direction if a candidate requires host capture policy, direct
guest mutation, raw guest memory, scan-code/layout policy, or a second
queue/run loop.

T248 is closed: it established the evidence-backed T249--T252 migration queue
and deferred raw host-clock observation. T249 now starts with its own S1
contract in [its task record](m5-t249-copied-input-ingress.md). Existing
unstarted hardware work begins at T253; completed task identities and artifact
revisions do not change.

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
