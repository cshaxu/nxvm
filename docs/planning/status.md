# Project Status

## Current Work

**Idle.** M5 Td S36 recorded the WASM media-backing admission boundary; admit
T284 only through a new approved packet.

## Current Technical Baseline

- **T283 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0283`. S2 media evidence and
  S4 HDD legacy-API closure passed; 42 static/governance gates and 110/110
  CTest passed. Artifact `nxvm_0_5_0283.exe` SHA-256:
  `A52B045DFF09CE37989267023D5AFDA43CA9CEDBED7AB0DDCD4DA32E370CEA17`.
- **Core boundary:** T243--T246 retain checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T276 | Moved the neutral FDC into `core_machine.shared_fdc`; composition only binds frozen PC/AT routes/media, while core owns controller lifecycle and the core-only FDC fixture. |
| T277 | Removed ATA PIO's direct `t_hdd` dependency and supplied the frozen media-registry boundary later consumed by T278. |
| T278 | Moved neutral ATA PIO into `core_machine.shared_hdc`; composition only binds frozen PC/AT routes/media, while the VM-free fixture proves IDENTIFY, DRQ, and IRQ14 acknowledgement. |
| T279 | Retired unbounded C formatting; source gate and corpus now enforce bounded output and explicit append capacity. |
| T280 | Made FDD/HDD candidate replacement atomic; HDD now preserves arbitrary raw byte length through virtual sector capacity, zero tail reads, and padded-tail persistence. |
| T281 | Renamed the sole core controller owners and configuration borrows from historical `shared_*` names to `fdc/hdc`, with no alias or behavior change. |
| T282 | Moved native window/console handle ownership from core to VM platform while preserving copied core input, presentation, and wait contracts. |
| T283 | Extended VM-free controller media evidence, then removed unused unsafe HDD CHS transfer state, corrected its capacity contract, and closed the regression class with a static gate. |

Detailed contracts, commands, artifact provenance, and prior closures are in
[M5 History](../history/m5.md) and Git history. The [M5 convergence queue]
(m5-pcat-hardware-convergence.md), [roadmap](roadmap.md), and
[M5 closure checklist](m5-closure-checklist.md) define current M5 scope; an
active packet becomes an operational authority only after approval.

## Recent Governance

- **M5 Td S30:** compacted completed M5 planning records and formalized the
  then-current T264--T278 forward queue.
- **M5 Td S31:** made `status.md` the sole current-baseline authority, retired
  completed T264--T266 records, and added a documentation-governance gate.
  This task changes no runtime behavior or active-task scope.
- **M5 Td S32:** admitted the original second core/composition migration.
- **M5 Td S33:** corrected it using the external-consumer boundary evidence:
  T271 admits only opaque policy-free host capabilities in `core/platform`,
  extends the migration through T278, and shifts the former forward queue to
  T279--T287. Filesystem, path, mount, and DOS policy remain above core.
- **M5 Td S34:** repaired the historical queue wording, inserted the approved
  T279--T283 reliability/interface package, and shifted the older future queue
  to T284--T292. This task changes no runtime behavior.
- **M5 Td S35:** tightened T279 bounded-format semantics, T280 raw-HDD commit
  truth, T282's existing-consumer-only host-surface audit, and T283's direct
  dependencies. This task changes no runtime behavior.
- **M5 Td S36:** recorded the WASM media-backing admission boundary without a
  runtime, API, artifact, or task-number change.
- **M5 Td S37:** made M5 history static, capped the status closure summary,
  and hardened the mojibake governance check. This task changes no runtime
  behavior or active-task scope.

## Milestone State

| Milestone | State | Current authority |
| --- | --- | --- |
| M0--M4 | Closed | [Roadmap](roadmap.md) and [history](../history/README.md) |
| M5 | Open | [M5 closure checklist](m5-closure-checklist.md) |
| M6--M8 | Not started | [Roadmap](roadmap.md) |
| M9 onward | Research queue | [Roadmap](roadmap.md) |

## Operational Reading Order

1. [Documentation Guide](../README.md)
2. [Project Status](status.md) and [M5 convergence queue](m5-pcat-hardware-convergence.md)
3. [M5 closure checklist](m5-closure-checklist.md)
4. [Architecture overview](../architecture/overview.md) and
   [contracts](../architecture/contracts.md)
5. [Requirements](../requirements/)
