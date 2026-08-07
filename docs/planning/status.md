# Project Status

## Current Work

**M5 T270 S3: Lossless Core Media Contract -- active.**

- **Original request:** replace the retained fixed single-slot block boundary
  with a frozen, multi-device, profile-neutral core media contract before any
  FDC/ATA controller migration.
- **Observed source baseline:** `core_machine_block_provider_slot` is a single
  HDD-geometry callback used by default-profile firmware; FDC directly owns
  `t_fdd` byte/CHS calls and HDC directly owns `t_hdd` bytes, geometry, and
  transfer cursor. Those are migration inputs, not alternate production paths.
- **S1 closed:** identity, capability, generation, copied byte and logical-
  sector I/O, geometry, format, flush, typed-failure semantics, legacy-slot
  removal, and the fake-provider fixture were frozen in the contracts.
- **S2 closed:** the frozen registry and core-only fake-provider fixture now
  prove multi-device binding, freeze, generation, byte/sector I/O, format,
  flush, absent/read-only/range/transient/permanent results, without rebinding
  production FDD/HDD paths.
- **S3 deliverable:** run the focused provider probe and retained NXVM matrix,
  add it to the current smoke set, then rebuild the T270 artifact and record
  source/verification/SHA evidence.
- **Rules:** no host paths, handles, DOS namespaces, topology defaults,
  controller migration, forwarding facade, private controller cache, or second
  media state. A provider is composition-owned and freezes before reset/run.
- **Evidence:** S1 governance and S2 GCC probe passed. S3 requires the full
  `current-gates-gcc` and CTest matrix, documentation governance, artifact SHA,
  and a final direct-source sweep confirming that the legacy block slot has
  neither gained a forwarding API nor a second media owner.
- **Stop:** stop and split if an operation needs mount/eject UI, persistence,
  path selection, firmware policy, or any host-filesystem policy.

## Current Technical Baseline

- **T269 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0269`; static/ownership
  checks and 102/102 CTest cases passed. Artifact `nxvm_0_5_0269.exe` SHA-256:
  `D7AF67A4D1E48D2065A246996487AFC1763E760354CD54F4712B2E88752BB80F`.
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
