# Project Status

## Current Work

**M5 T278 S1: Neutral ATA PIO Controller Migration -- active.**

- **Original request:** move the now-neutral ATA PIO controller into
  `core/machine`; VM retains only PC/AT topology, frozen media policy, profile
  firmware, boot policy, and product behavior.
- **S1 deliverable:** freeze the core ATA configuration/provider surface,
  audit VM-specific identifiers, and define a core-only ATA fixture.
- **S1 complete:** after T277, `hdc.c` depends only on core media/PIC/port
  contracts. Its mutable task-file, PIO buffer, phase, reset, IRQ14 and copied
  geometry behavior may move together. The default profile retains the
  primary-master ports/IRQ/LBA28 declaration, selected HDD identity, and ROM
  INT 13h firmware. The core connection takes only registry/id, PIC route and
  explicit ATA port/config fields; it has no PC/AT default or host policy.
- **S2 admission:** move `hdc.[ch]` as one unit, store the controller in
  `core_machine`, bind it only during configuration, and have core own reset,
  refresh and teardown. All retained VM tests must use the one core instance.
- **S3 evidence:** a VM-free ATA fixture must bind fake media/PIC/ports and
  prove IDENTIFY or one bounded PIO transaction; retain ATA port, DOS and HDD
  boot corpus plus current gates.
- **Rules:** core owns the one controller state, IRQ14 source lifecycle, and
  port protocol after migration. Do not add ATA DMA, new commands, host I/O,
  a second controller, or profile defaults in core.
- **Stop:** stop and split if the move needs a core -> VM include, a direct
  backing object, PC/AT default, firmware shortcut, or changed boot/UI path.

## Current Technical Baseline

- **T277 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0277`; static/ownership
  checks and 108/108 CTest cases passed. Artifact `nxvm_0_5_0277.exe` SHA-256:
  `EA52F19ED01338A46BAEF1A3F20432136605A30F64C7EDC2B0CC4E47FB00E0CE`.
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
| T277 | Removed ATA PIO's direct `t_hdd` dependency; it now consumes frozen HDD media registry query/read/write data while retaining its VM controller location until T278. |

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
