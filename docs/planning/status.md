# Project Status

## Current Work

No implementation subtask is active. T272 may begin only with its own approved
VM media-adapter/rebinding packet.

## Current Technical Baseline

- **T271 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0271`; static/ownership
  checks and 104/104 CTest cases passed. Artifact `nxvm_0_5_0271.exe` SHA-256:
  `9B743CEB1B88D02835BD36C8B09D5EA7436B2110ED71EA2E9D806F14D23DDA1C`.
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
