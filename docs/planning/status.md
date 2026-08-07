# Project Status

## Current Work

**M5 T280 S2: Atomic Media Implementation -- active.**

- **Original request:** make FDD/HDD replacement atomic and admit arbitrary
  raw-HDD byte lengths. A failed candidate must leave every old-media field
  published and unchanged; HDD virtual capacity is `ceil(raw_bytes / 512) *
  512`, with a zero-filled partial tail sector.
- **S1 closed:** the inventory confirms `vm/machine/fdd` and `vm/machine/hdd`
  own their backing bytes and replacement state; core FDC/HDC only retain
  frozen provider bindings. Candidate population precedes the one state
  publish; backing bytes have no separately closeable old-media handle, while
  explicit persistence must write and close before it changes the mount.
- **S2 deliverable:** implement the frozen candidate/commit/rollback boundary,
  raw-versus-virtual HDD capacity, tail persistence, and the focused corpus.
- **Rules:** VM backing objects retain path, mount, persistence, and geometry
  policy; controllers continue through frozen media providers only. No ATA
  command expansion, DMA, dynamic topology, FDD geometry broadening, or host
  shortcut is in scope.
- **Similar-issue sweep:** inspect all FDD/HDD load/save, allocation, file I/O,
  provider query/read/write/flush, and replacement/reset paths. Classify every
  production failure path as fixed, safe, or deferred.
- **Evidence:** focused 0/1/511/512/513-byte HDD and failed-replacement
  corpus; retained FDD/HDD boot, ATA PIO, FDC READ TRACK, Console/debugger,
  current GCC/CTest gates; artifact `build/output/nxvm_0_5_0280.exe`.
- **Stop:** stop if atomic replacement needs a second media/controller state,
  host-clock behavior, FDD geometry expansion, or a VM bypass of core media.

## Current Technical Baseline

- **T279 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0279`; static/ownership
  checks and 110/110 CTest cases passed. Artifact `nxvm_0_5_0279.exe` SHA-256:
  `F39286FEA7A6F339711BCD5206C958D9B79D195C595CA367FDC355AD48A56AC0`.
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
