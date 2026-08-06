# Project Status

## Current Work

**M5 idle after T253 S3: next task awaits an approved packet.**

T253 closes only multi-sector ATA PIO task-file count progression. IDE DMA,
ATAPI, LBA48, cache, secondary channels, slave devices, extra topology, and
host-clock timing remain separate admissions.

| Closure | Evidence |
| --- | --- |
| T249--T251 | Copied input, presentation, and cancellable wait contracts are closed with artifacts `0.5.0248`--`0.5.0250`. |
| T252 | Composition owns the sole run-handle teardown sequence; artifact `0.5.0251` and 86/86 current CTest pass. |
| T253 | ATA PIO sector-count progression is controller-owned; artifact `0.5.0252` and 87/87 current CTest pass. |

This explicit idle state is permitted only after a completed task closes and
before a new task packet is approved. Existing unstarted hardware work begins
at T254; completed task identities and artifact revisions do not change.

## Current Technical Baseline

- **T253 S3:** `current-gcc` and `verify-current-artifact-target` select
  `vm-0-5-0252`; static/ownership checks and 87/87 CTest cases passed.
  Artifact `nxvm_0_5_0252.exe` SHA-256:
  `B565B73D1C1E5EB77C04549944AB28EA2CF6DE59AB2FD2B087D957CAEB020A87`.
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
- **M5 Td S25:** reconciled completed M5 baseline references to T252 S3 and
  removed stale task scheduling from the completed T248 record. This
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
