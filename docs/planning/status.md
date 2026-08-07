# Project Status

## Current Work

**M5 T273 S2: Neutral MC146818 Mechanism Migration -- active.**

- **Original request:** move the register/calendar/tick/IRQ-output mechanism
  into `core/machine`; retain VM-only NMI policy, defaults, ROM firmware, and
  PC/AT route selection.
- **Audit:** the mutable controller currently combines neutral MC146818 state
  with one VM callback: port 70h bit 7 calls `core_machine_set_nmi_mask`.
  Its `VCMOS_POST`/INT assembly strings are default-ROM firmware, not device
  mechanism, and must stay under the VM profile.
- **S1 complete:** froze `core_machine_rtc` state/config contract, VM NMI
  adapter, profile defaults/NVRAM boundary, and the retained RTC port corpus.
- **S2 deliverable:** move the neutral mechanism with `git mv`, split default
  ROM assembly strings into profile firmware, bind the PC/AT 70h/71h provider
  from composition, and add a core-only RTC probe. Keep the session lifecycle
  and sole CPU executor unchanged.
- **Rules:** core does not know PC/AT port defaults, NMI, BDA, BIOS, host time,
  or firmware. VM may extract port 70h bit 7 then delegate only index/data;
  it may not mirror RTC register/calendar/IRQ state.
- **Evidence:** current CMOS source/test inventory plus documentation
  governance. S2 uses `git mv`, splits ROM strings from device code, and keeps
  the current session lifecycle/run path intact.
- **Stop:** stop and split if migration needs a core -> VM dependency, a
  second RTC state object, profile-specific controller default, or host clock.

## Current Technical Baseline

- **T272 artifact identity:** `current-gcc` and
  `verify-current-artifact-target` select `vm-0-5-0272`; static/ownership
  checks and 105/105 CTest cases passed. Artifact `nxvm_0_5_0272.exe` SHA-256:
  `3E4CF6782886C043E797EBECA4D6A4EB61399660E3F13291673B8696B679F99E`.
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
