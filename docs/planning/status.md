# Project Status

## Current Work

**M5 T250 S3 active: verify copied presentation and artifact.**

Original request: execute the second admitted T248 migration without changing
NXVM display behavior. S1 fixed the copied mailbox contract, producer/consumer
inventory, and shutdown rule. S2 moved only the copy container to
`core/platform`; the snapshot route and VM renderer policy remain intact. S3
now verifies all paths and allocates artifact revision `0.5.0249`.

| Requirement | S3 evidence |
| --- | --- |
| Focused boundary | Exercise independent frame copies and post-finalize rejection; verify producers/consumers only see copied data. |
| Full regression | Run current GCC/CTest and retained Console/window, text/CGA/EGA, DOS, debugger, FDD/HDD regressions. |
| Artifact | Build `nxvm_0_5_0249.exe`, record SHA/banner/source commit, then close T250. |

Applicable rules: module layout, contracts, coding/source policy, execution
workflow, and one active subtask. Planned commands:
`rg -n "vm_platform_presentation_mailbox|vm/platform/presentation_mailbox|presentation_mailbox\\.c|vm-platform-presentation-mailbox" src tests cmake CMakeLists.txt`, the focused mailbox smoke, and `current-gates-gcc`.
Expected marker: `M5:T250:S3:PRESENTATION-BOUNDARY-CLOSED:OK`. Stop for owner
direction if the contract requires guest VRAM, renderer ownership, display
policy, or a second presentation path.

T248 is closed: it established the evidence-backed T249--T252 migration queue
and deferred raw host-clock observation. T249 is closed with artifact
`0.5.0248`; T250 now starts with its own S1 contract in
[its task record](m5-t250-presentation-mailbox.md). Existing unstarted
hardware work begins at T253; completed task identities and artifact revisions
do not change.

## Current Technical Baseline

- **T249 S3:** `current-gcc` and `verify-current-artifact-target` select
  `vm-0-5-0248`; static/ownership checks and 85/85 CTest cases passed.
  Artifact `nxvm_0_5_0248.exe` SHA-256:
  `6B0CAD6A3DB97D794963E052E3A70C08C2080614047AB7AC5355CCE6E07E8F35`.
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
