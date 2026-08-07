# Project Status

## Current Work

**M5 T283 S1: Core Controller Media-I/O Corpus Contract -- active.**

- **Original request:** extend VM-free core FDC/ATA fixtures so they execute
  real provider query/read/write and applicable format/flush operations;
  verify typed absent/short/read-only/read-write failures, generation change,
  and frozen-registry rebind rejection without new controller commands or
  test-only runtime paths.
- **S1 deliverable:** inventory current core FDC/HDC fixture provider usage and
  controller-visible result mapping; freeze a compact fake provider corpus with
  real callback counters/data and each required typed failure. **Complete:**
  FDC currently proves only `SPECIFY`; HDC proves only `IDENTIFY` plus one
  read callback.  The media contract is all-or-typed-failure, so “short I/O”
  means the existing `invalid-range` rejection, not a new partial-transfer API.
- **S2/S3:** add only owner-built fixture evidence, run focused and retained DOS
  regressions, then build `nxvm_0_5_0283.exe`, record SHA-256, and close.
- **Rules:** core controllers continue through frozen `core_machine_media`
  bindings only. No VM/firmware shortcut, topology change, new ATA/FDC command,
  or second media route is in scope.
- **Similar-issue sweep:** inspect all FDC/HDC provider calls and existing core
  fixtures for fake success paths, ignored result kinds, generation caching, or
  mutable registry rebinds.
- **Stop:** stop if a needed assertion requires VM media backing, a controller
  command-family expansion, dynamic topology, or a test-only execution loop.

## Current Technical Baseline

- **T282 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0282`; focused host-surface
  and full gate evidence are retained. Artifact `nxvm_0_5_0282.exe` SHA-256:
  `F967C5E2766000200BD8E40A220E448D6CBE0E1E6756403A06892955240632F4`.
- **Core boundary:** T243--T246 retain checked physical memory, bounded `#UD`
  transitions, immutable ROM mapping, and atomic real-mode entry plans.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T264 | Closed PC/AT ownership debt without changing retained NXVM behavior. |
| T265 | Added bounded Level 2 instruction-cost attribution with zero default-PC/AT surcharges pending calibration evidence. |
| T266 | Closed bounded VADP CRTC cursor/page/status behavior and copied column/row coordinates. |
| T267 | Added bounded standard PS/2 AUX sample-rate, resolution, and status commands through the core KBC and IRQ12 guest path. |
| T268 | Required DOR.ME0 for FDC drive-0 readiness while retaining FDC/DMA2/IRQ6 ownership and the boot path. |
| T269 | Directly closed block, demand, single, and M2M one-grant semantics; two run quantums replay the same FDC DMA2 DOS result. |
| T270 | Added the frozen multi-device core media contract and fake-provider corpus; retained the old single block slot only as T272's explicitly bounded migration source. |
| T271 | Added a synchronous opaque core/platform backing resource with one close owner; existing copied input and cancellation contracts remain the only such facilities. |
| T272 | Replaced the old single-slot block bridge with frozen FDD/HDD media providers and copied ROM geometry; FDC/HDC direct backing use remains explicitly deferred to T275/T277. |
| T273 | Moved the neutral MC146818 register/calendar/tick/IRQ mechanism into core; VM retains profile NVRAM defaults and the PC/AT 70h bit-7 NMI/71h port adapter. |
| T274 | Added a core-only fixture proving one machine can bind neutral RTC/media/backing providers, freeze, reset, apply an entry plan, and run a bounded slice without VM vocabulary. |
| T275 | Decoupled the retained FDC controller from `t_fdd`; it now consumes only frozen media provider operations while retaining its single DMA2/IRQ6 state machine. |
| T276 | Moved the neutral FDC into `core_machine.shared_fdc`; composition only binds frozen PC/AT routes/media, while core owns controller lifecycle and the core-only FDC fixture. |
| T277 | Removed ATA PIO's direct `t_hdd` dependency and supplied the frozen media-registry boundary later consumed by T278. |
| T278 | Moved neutral ATA PIO into `core_machine.shared_hdc`; composition only binds frozen PC/AT routes/media, while the VM-free fixture proves IDENTIFY, DRQ, and IRQ14 acknowledgement. |
| T279 | Retired unbounded C formatting; source gate and corpus now enforce bounded output and explicit append capacity. |
| T280 | Made FDD/HDD candidate replacement atomic; HDD now preserves arbitrary raw byte length through virtual sector capacity, zero tail reads, and padded-tail persistence. |
| T281 | Renamed the sole core controller owners and configuration borrows from historical `shared_*` names to `fdc/hdc`, with no alias or behavior change. |

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
